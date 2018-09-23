#pragma once

#include <pthread.h>
#include <stddef.h>

struct AbobCloudServer {
  pthread_t img_serv;
  pthread_t flag_serv;

  int tcp_fd;
  int udp_fd;

  // 1 bit - should stop
  int flags;
};

struct AbobCloudServer *init(short port);
void start(struct AbobCloudServer *);
void stop(struct AbobCloudServer *);
void run(struct AbobCloudServer *);
