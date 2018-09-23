#include "net.h"
#include <signal.h>

struct AbobCloudServer *server;

void intHandler() {
  stop(server);
}


int main() {
  short port = 8080;
  server = init(port);
  start(server);

  signal(SIGINT, intHandler);

  run(server);
  return 0;
}
