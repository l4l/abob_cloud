#include "flag.h"

#include "upng/upng.h"

void retrieve_flag(const uint32_t *buf, char *flag, unsigned width, unsigned height) {
  // assert(FLAG_SIZE == 4 * height);
  // assert(FLAG_SIZE == width);
  memset(flag, 0, FLAG_SIZE);

  // Loop will fold matrix columns via xor resulting in a vector
  // E.g:
  // buf = 1 0 1     0
  //       0 1 0  -> 1
  //       1 1 0     0
  // Note, that endian doesn't matter, so should work same on all platforms
  for (unsigned i = 0; i < width; ++i) {
    for (unsigned j = 0; j < height; ++j) {
      uint32_t col = buf[i * height + j];
      flag[4 * j + 0] ^= (uint8_t)(col);
      flag[4 * j + 1] ^= (uint8_t)(col >> 8);
      flag[4 * j + 2] ^= (uint8_t)(col >> 16);
      flag[4 * j + 3] ^= (uint8_t)(col >> 24);
    }
  }

  // printf("Flag: ");
  // for (unsigned i = 0; i < width; ++i) {
  //   printf("%02hhx", flag[i]);
  // }
  // printf("\n");
}
