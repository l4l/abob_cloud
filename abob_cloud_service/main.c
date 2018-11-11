#include "net.h"
#include <signal.h>

struct AbobCloudServer *server;

void intHandler() {
  stop(server);
}


int main() {
  short port = 8080;
  server = init(port);
  printf("AbobCloud initialized\n");
  start(server);

  signal(SIGCHLD, SIG_IGN);
  signal(SIGINT, intHandler);

  run(server);
  clean(server);
  return 0;
}
