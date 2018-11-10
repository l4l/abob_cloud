#include "flag.h"

#include "upng/upng.h"

void retrieve_flag(const uint32_t *buf, char *flag, unsigned width, unsigned height) {
  memset(flag, 0, FLAG_SIZE);

  // Loop will fold matrix columns via xor resulting in a vector
  // E.g:
  // buf = 1 0 1
  //       0 1 0
  //       1 1 0
  // Will result to:
  // col = 0 0 1
  // Vector components then folded to the flag values
  // col = 0x12345678 => flag ^= 0x12 ^ 0x34 ^ 0x56 ^ 0x78
  // Note, that endian doesn't matter, so should work same on all platforms
  for (unsigned i = 0; i < width; ++i) {
    for (unsigned j = 0; j < height; ++j) {
      uint32_t col = buf[i * height + j];
      flag[i] ^= (uint8_t)(col)
              ^  (uint8_t)(col >> 8)
              ^  (uint8_t)(col >> 16)
              ^  (uint8_t)(col >> 24);
    }
  }

  printf("Flag: ");
  for (unsigned i = 0; i < width; ++i) {
    printf("%02hhx", flag[i]);
  }
  printf("\n");
}
