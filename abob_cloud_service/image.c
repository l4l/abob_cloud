#include "image.h"

#include <stdlib.h>

struct Image *new_img(size_t len) {
  struct Image *img = malloc(sizeof(size_t) + sizeof(len));
  img->len = len;
  return img;
}

void free_img(struct Image *img) {
  free(img);
}
