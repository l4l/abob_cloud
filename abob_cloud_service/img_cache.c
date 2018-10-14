#include "img_cache.h"
#include "img_db.h"

#define CACHE_SIZE 48

typedef struct {
  char flag[FLAG_SIZE];
  size_t len;
  char data[MAX_IMG];
} ImageBuf;

/**
 * Simple ring buffer implementation without priorities
 * Head points to the next place for insertion
 * Size represent amount of elements currently presented in buffer, so that:
 * size <= CACHE_SIZE
 */
static ImageBuf buf_images[CACHE_SIZE];
static char zeroes_filler[1024];
static struct Hash buf_hashes[CACHE_SIZE];
static size_t head = 0;
static size_t size = 0;

void cache_add(const struct Hash *hash, struct Image *img) {
  (void)zeroes_filler;
  memcpy(&buf_images[head], img, size_of(img));
  memcpy(&buf_hashes[head], hash, sizeof(struct Hash));

  if (size < CACHE_SIZE) {
    size++;
  }
  head++;
  if (head >= CACHE_SIZE) {
    head = 0;
  }
}

struct Image *cache_find(const struct Hash *h) {
  struct Image *found = NULL;
  for (size_t i = 0; i < size; ++i) {
    if (memcmp(h, &buf_hashes[i], sizeof(struct Hash)) == 0) {
      found = clone_img((struct Image*)&buf_images[i]);
      break;
    }
  }

  return found;
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
