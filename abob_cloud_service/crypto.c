#include "crypto.h"
#include "SHA256/sha256.h"

struct Hash make_hash(size_t len, char *data) {
  struct Hash h;
  sha256_context context;
  sha256_init(&context);
  sha256_hash(&context, data, len);
  sha256_done(&context, h.data);
  return h;
}
