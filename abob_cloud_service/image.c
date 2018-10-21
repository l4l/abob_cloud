#include "image.h"

#include <stdlib.h>

struct Image *new_img(size_t len) {
  struct Image *img = malloc(FLAG_SIZE + sizeof(len) + len);
  img->len = len;
  return img;
}

struct Image *clone_img(struct Image *img) {
  struct Image *new_img = malloc(size_of(img));
  new_img->len = img->len;
  memcpy(new_img->flag, img->flag, FLAG_SIZE);
  memcpy(new_img->data, img->data, img->len);
  // make sure that printf(data) won't go to far
  new_img->data[img->len] = 0;
  return new_img;
}

size_t size_of(struct Image *img) {
  return FLAG_SIZE + sizeof(img->len) + img->len + 1;
}

void free_img(struct Image *img) {
  free(img);
}
