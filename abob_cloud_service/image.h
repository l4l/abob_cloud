#pragma once

#include "common.h"

#define IMG_WIDTH 512
#define IMG_HEIGHT 128

#pragma pack(push, 1)
struct Image {
  char flag[FLAG_SIZE];
  size_t len;
  char data[];
};
#pragma pack(pop)

struct Image *new_img(size_t len);
size_t size_of(struct Image *);
void free_img(struct Image *);
