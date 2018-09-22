#include "common.h"
#include "image.h"
#include "img_db.h"
#include "net.h"
#include "serv.h"
#include <signal.h>

int *flags;

void intHandler(int v) {
  *flags &= ~1; 
}

int main() {
  init_db();
  struct AbobCloudServer server = {};
  server.tcp_fd = create_tcp(8080);
  server.udp_fd = create_udp(8080);

  start(&server);

  flags = &server.flags;
  signal(SIGINT, intHandler);

  while ((server.flags & 1) == 0) sleep(1);

  close_db();
  return 0;
}
