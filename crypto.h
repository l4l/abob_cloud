#pragma once

#include <stddef.h>

#define HASH_SIZE 32

struct Hash {
  char data[HASH_SIZE];
};

struct Hash make_hash(size_t len, char *data);
