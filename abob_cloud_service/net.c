#include "net.h"
#include "common.h"
#include "crypto.h"
#include "flag.h"
#include "img_db.h"
#include "net.h"

#define __USE_MISC

#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int create_socket_common(short port, int type) {
  struct sockaddr_in serv_addr;

  int sockfd = socket(AF_INET, type, 0);
  if (sockfd < 0) {
    die("at opening socket");
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    die("at addr binding");
  }
  return sockfd;
}

static int create_tcp(short port) {
  int fd = create_socket_common(port, SOCK_STREAM);

  if (listen(fd, 5) != 0) {
    die("at listening");
  }

  return fd;
}

static int create_udp(short port) { return create_socket_common(port, SOCK_DGRAM); }

static inline void handle_upload(int fd) {
  size_t size;
  if (read(fd, &size, sizeof(size)) != sizeof(size) || size == 0 ||
      size > 4096) {
    printf("[INFO] Unknown request\n");
    goto end;
  }

  struct Image *img = new_img(size);
  if (read(fd, img->data, size) != (int)size) {
    printf("[INFO] Unknown request\n");
  } else {
    struct Hash h = make_hash(img->len, img->data + sizeof(img->len));
    add(&h, img);
  }

  free_img(img);

end:
  shutdown(fd, SHUT_RDWR);
  exit(0);
}

static void *img_server_main_loop(void *v) {
  struct AbobCloudServer *ptr = v;
  while ((ptr->flags & 1) == 0) {
    int new_fd = accept(ptr->tcp_fd, NULL, NULL);
    int pid = vfork();
    if (pid < 0) {
      die("Not able to make a fork\n");
    }
    if (pid == 0) {
      handle_upload(new_fd);
    }
  }
  pthread_exit(v);
}

static void *flag_server_main_loop(void *v) {
  struct AbobCloudServer *ptr = v;
  struct Hash hash;
  struct sockaddr_in addr;
  size_t addr_len;
  uint8_t flag[FLAG_SIZE];

  while ((ptr->flags & 1) == 0) {
    ssize_t n = recvfrom(ptr->udp_fd, hash.data, HASH_SIZE, 0,
                         (struct sockaddr *)&addr, &addr_len);
    if (n < 0) {
      printf("[ERROR] at recvfrom");
      continue;
    }

    if (n != HASH_SIZE) {
      printf("[WARN] received less data, than assumed: %d\n", n);
      continue;
    }

    struct Image *img = search(&hash);
    if (img == NULL) {
      printf("[WARN] image isn't found, skipping\n");
      continue;
    }

    retrieve_flag(img, flag);
    free_img(img);
    sendto(ptr->udp_fd, flag, FLAG_SIZE, 0, (struct sockaddr *)&addr,
           sizeof(addr));
  }
  pthread_exit(v);
}

struct AbobCloudServer *init(short port) {
  init_db();

  struct AbobCloudServer *server = malloc(sizeof(struct AbobCloudServer));
  memset(server, 0, sizeof(struct AbobCloudServer));

  server->tcp_fd = create_tcp(port);
  server->udp_fd = create_udp(port);

  return server;
}

void clean(struct AbobCloudServer *server) {
  free(server);
}

void start(struct AbobCloudServer *server) {

  pthread_create(&server->img_serv, NULL, &img_server_main_loop, server);
  pthread_create(&server->flag_serv, NULL, &flag_server_main_loop, server);

  pthread_detach(server->img_serv);
  pthread_detach(server->flag_serv);
}

void stop(struct AbobCloudServer *server) {
  server->flags &= ~1;
  close_db();
}

void run(struct AbobCloudServer *server) {
  while ((server->flags & 1) == 0) sleep(1);
}