#include <pthread.h>

#include "upng/upng.h"

#include "img_cache.h"
#include "img_db.h"
#include "flag.h"

#define CACHE_SIZE 48

struct HashItem {
  struct Hash hash;
  size_t offset;
};

/**
 * Memory efficient ring buffer implementation without priorities
 * Head points to the next place for insertion
 * Nexptr represents offset to the next available place
 * Size represent amount of elements currently presented in buffer, so that:
 * size <= CACHE_SIZE
 */
// static ImageBuf buf_images[CACHE_SIZE] = {0};
static char buf_images[CACHE_SIZE * (sizeof(struct Image) + MAX_IMG)] = {0};
static char zeroes_filler[MAX_IMG];
static struct HashItem buf_hashes[CACHE_SIZE] = {0};
static size_t head = 0;
static size_t next_ptr = 0;
static size_t size = 0;
static pthread_mutex_t cache_mutex;

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static void cache_dump() {
  printf("Cache capacity: %d, head: %d, next_ptr: %d, size: %d\n", CACHE_SIZE, head, next_ptr, size);
  for (int i = 0; i < CACHE_SIZE; ++i) {
    printf("cache[%2d]: flag=", i);
    for (int j = 0; j < CACHE_SIZE; ++j) {
      printf("%02hhx", ((struct Image*)&buf_images[buf_hashes[i].offset])->flag[j]);
    }
    printf("\n");
  }
}

static void cache_add(const struct Hash *hash, struct Image *img) {
  (void)zeroes_filler;
  upng_t *png = upng_new_from_bytes((unsigned char*)img->data, img->len);
  if (upng_decode(png) != UPNG_EOK) {
    printf("[WARN] Tried to acquire flag from non-png file\n");
    upng_free(png);
    return;
  }
  const unsigned width = upng_get_width(png), height = upng_get_height(png);
  // printf("width: %d, height: %d\n", width, height);
  if (width > IMG_WIDTH || height > IMG_HEIGHT) {
    printf("[WARN] image size aren't correct\n");
    upng_free(png);
    return;
  }

  // printf("Cache before update\n");
  // cache_dump();

  pthread_mutex_lock(&cache_mutex);

  const size_t current_ptr = next_ptr;
  struct Image *current = (struct Image *)&buf_images[current_ptr];

  // this cast may cause overflow, since picture bitness isn't checked
  const uint32_t* buf = upng_get_buffer(png);
  const size_t len = upng_get_size(png);
  current->len = len;
  memcpy(current->data, buf, len);
  upng_free(png);

  retrieve_flag(current->data, img->flag, width, height);
  memcpy(current->flag, img->flag, FLAG_SIZE);

  next_ptr += sizeof(current->flag) + sizeof(current->len) + current->len;

  buf_hashes[head].offset = current_ptr;
  memcpy(&buf_hashes[head].hash, hash, HEX_HASH_SIZE);

  // TODO: check for the following overwritings

  if (size < CACHE_SIZE) {
    size++;
  }
  head++;
  if (head >= CACHE_SIZE) {
    head = 0;
    next_ptr = 0;
  }

  pthread_mutex_unlock(&cache_mutex);

  // printf("Cache after update\n");
  // cache_dump();
}

static struct Image *cache_find(const struct Hash *h) {
  for (size_t i = 0; i < size; ++i) {
    if (memcmp(h, &buf_hashes[i], HASH_SIZE) == 0) {
      return (struct Image*)&buf_images[i];
    }
  }
  printf("[INFO] cache miss\n");

  return NULL;
}

void search_destruct(struct Image *img) {
  if ((char*)img >= buf_images && (char*)img <= buf_images + CACHE_SIZE) {
    // in cache, no dtor is needed
  } else {
    free_img(img);
  }
}

struct Image *search(const struct Hash *h) {
  struct Image *img = cache_find(h);

  if (img == NULL) {
    img = db_search(h);
    if (img != NULL) {
      printf("[INFO] adding item to cache\n");
      cache_add(h, img);
    }
  } else {
    printf("[INFO] img found in cache\n");
  }

  return img;
}
