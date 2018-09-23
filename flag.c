#include "flag.h"

#include "upng/upng.h"

void retrieve_flag(struct Image *img, unsigned char *flag) {
  memset(flag, 0, FLAG_SIZE);
  upng_t *png = upng_new_from_bytes((unsigned char*)img->data, img->len);
  if (upng_decode(png) != UPNG_EOK) {
    goto end;
  }
  const unsigned width = upng_get_width(png), height = upng_get_height(png);
  if (width != FLAG_SIZE || height != FLAG_SIZE) {
    goto end;
  }
  uint32_t *column_count = malloc(width * sizeof(uint32_t));
  memset(column_count, 0, width);

  // this cast may cause overflow, since picture bitness isn't checked
  const uint32_t* buf = upng_get_buffer(png);
  for (unsigned i = 0; i < width; ++i) {
    for (unsigned j = 0; j < height; ++j) {
      column_count[i] ^= buf[i * width + j];
    }
  }

  for (unsigned i = 0; i < width; ++i) {
    uint32_t col = column_count[i];
    flag[i] = (uint8_t)(col & 0xff) ^ (uint8_t)((col >> 8) & 0xff) ^
              (uint8_t)((col >> 16) & 0xff) ^ (uint8_t)((col >> 24) & 0xff);
  }

end:
  upng_free(png);
}
