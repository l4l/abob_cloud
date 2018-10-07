#pragma once

#include <stddef.h>

#define HASH_SIZE 32
#define HEX_HASH_SIZE (HASH_SIZE * 2)

struct Hash {
  char data[HEX_HASH_SIZE + 1];
};

struct Hash make_hash(char *data, size_t len);
