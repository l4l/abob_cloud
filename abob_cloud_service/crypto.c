#include "crypto.h"
#include <stdio.h>
#include "SHA256/sha256.h"

static void hexlify(const unsigned char *hash, char *hex) {
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    sprintf(hex + i * 2, "%02x", hash[i]);
  }
}

struct Hash make_hash(char *data, size_t len) {
  unsigned char raw_hash[HASH_SIZE];
  sha256_context context;
  sha256_init(&context);
  sha256_hash(&context, data, len);
  sha256_done(&context, raw_hash);
  struct Hash h = {0};
  hexlify(raw_hash, h.data);
  return h;
}
