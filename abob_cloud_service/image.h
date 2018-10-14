#pragma once

#include "common.h"

#pragma pack(push, 1)
struct Image {
  char flag[FLAG_SIZE];
  size_t len;
  char data[];
};
#pragma pack(pop)

struct Image *new_img(size_t len);
struct Image *clone_img(struct Image *img);
size_t size_of(struct Image *);
void free_img(struct Image *);
