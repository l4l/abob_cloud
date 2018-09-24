#pragma once

#include "common.h"

struct Image {
  size_t len;
  char data[];
};

struct Image *new_img(size_t len);
void free_img(struct Image *);
