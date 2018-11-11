#include "image.h"

#include <stdlib.h>

static inline size_t img_size_of(size_t len) {
  return FLAG_SIZE + sizeof(len) + len + 1;
}

struct Image *new_img(size_t len) {
  struct Image *img = calloc(1, img_size_of(len));
  img->len = len;
  return img;
}

size_t size_of(struct Image *img) {
  return img_size_of(img->len);
}

void free_img(struct Image *img) {
  free(img);
}
