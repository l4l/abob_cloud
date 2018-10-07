#include "common.h"
#include <stdarg.h>
#include <errno.h>

void die(char *format, ...) {
  printf("[FATAL] errno: %s, msg: ", strerror(errno));
  va_list args;
  va_start(args, format);

  vprintf(format, args);

  va_end(args);
  puts("");
  exit(1);
}
