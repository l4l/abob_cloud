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
void free_img(struct Image *);
