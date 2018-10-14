#include "flag.h"

#include "upng/upng.h"

void retrieve_flag(struct Image *img) {
  memset(img->flag, 0, FLAG_SIZE);
  upng_t *png = upng_new_from_bytes((unsigned char*)img->data, img->len);
  if (upng_decode(png) != UPNG_EOK) {
    printf("[WARN] Tried to acquire flag from non-png file\n");
    goto end;
  }
  const unsigned width = upng_get_width(png), height = upng_get_height(png);
  if (width != FLAG_SIZE || height != FLAG_SIZE) {
    printf("[WARN] image size aren't correct\n");
    goto end;
  }
  uint32_t *column_count = malloc(width * sizeof(uint32_t));
  memset(column_count, 0, width);

  // this cast may cause overflow, since picture bitness isn't checked
  const uint32_t* buf = upng_get_buffer(png);

  // Loop will fold matrix columns via xor resulting in a vector
  // E.g:
  //      1 0 1
  //      0 1 0
  //      1 1 0
  // Will result to:
  //      0 0 1
  for (unsigned i = 0; i < width; ++i) {
    for (unsigned j = 0; j < height; ++j) {
      column_count[i] ^= buf[i * height + j];
    }
  }

  // Fold the vector components to the flag values
  // e.g uint32_t: 0x12345678
  // Will be converted to: 0x12 ^ 0x34 ^ 0x56 ^ 0x78 = 0x08
  // Note, that endian doesn't matter, so should work same on all platforms
  for (unsigned i = 0; i < width; ++i) {
    uint32_t col = column_count[i];
    img->flag[i] = (uint8_t)(col & 0xff) ^ (uint8_t)((col >> 8) & 0xff) ^
              (uint8_t)((col >> 16) & 0xff) ^ (uint8_t)((col >> 24) & 0xff);
  }

end:
  upng_free(png);
}
