#pragma once

#include "easy-ecc/ecc.h"

#include <stddef.h>

struct Hash {
  char data[ECC_BYTES];
};

struct Hash make_hash(size_t len, char *data);
