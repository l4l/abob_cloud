#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define KEY_HEX_SIZE 32
#define KEY_SIZE (KEY_HEX_SIZE / 2)

/*

    ~ Loading flow ~


Binary placed in a section
License key got from input
Binary read from section andxored with license key
The result is written to newly created file in temp folder
New binary launched via execv
After binary is dead (waipid) temporary executable is deleted

*/

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    printf("Your license key expired, please enter a new one\n"
           "%s <license_key>\n",
           argv[0]);
    return 1;
  }

  int is_invalid = strlen(argv[1]) != KEY_HEX_SIZE;
  for (int i = 0; !is_invalid && i < KEY_HEX_SIZE; ++i) {
    if (!isalnum(argv[1][i])) {
      is_invalid = 1;
    }
  }

  if (is_invalid) {
    printf("License key has invalid format, "
           "please double check it from your floppy\n");
    return 2;
  }

  char key[KEY_SIZE];
  const char *ptr = argv[1];
  for (int i = 0; i < KEY_SIZE; ++i) {
    sscanf(ptr, "%2hhx", &key[i]);
    ptr += 2;
  }

  // 421bee0d7d772f5489192ae430037ac0
  size_t xor = 0, sum = 0, xor1337 = 0;
  for (int i = 0; i < KEY_SIZE; ++i) {
    xor ^= key[i];
    sum += key[i];
    xor1337 = (xor1337 ^ key[i]) % 1337;
  }

  if (xor != 28 || sum != 492 || xor1337 != 52) {
    printf("License key has invalid format, "
           "please double check it from your floppy\n");
    return 3;
  }

  const size_t sgmt_size = 0xac1c, sgmt_offset = 0x307f;

  // Can be read directly from current loaded binary
  // but i'm too lazy to parse elf, sry
  FILE* self = fopen(argv[0], "rb");
  char *buf = malloc(sgmt_size);
  fseek(self, sgmt_offset, SEEK_SET);
  fread(buf, sgmt_size, 1, self);
  fclose(self);

  for (size_t i = 0; i < sgmt_size; ++i) {
    buf[i] ^= key[i % KEY_SIZE];
  }

  int f = open("/tmp/abob_cloud", O_WRONLY | O_CREAT, S_IRWXU);
  write(f, buf, sgmt_size);
  close(f);
  free(buf);

  printf("Binary is unpacked and available at /tmp/abob_cloud\n");
  return 0;
}
