#pragma once

#include <pthread.h>

struct AbobCloudServer {
  pthread_t img_serv;
  pthread_t flag_serv;

  int tcp_fd;
  int udp_fd;

  // 1 bit - should stop
  int flags;
};

void start(struct AbobCloudServer *);
