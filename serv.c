#include "serv.h"
#include "crypto.h"
#include "flag.h"
#include "img_db.h"
#include "net.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static inline void handle_upload(int fd) {
  const size_t BUF_SIZE = sizeof(size_t);
  size_t size;
  if (read(fd, &size, sizeof(size)) != sizeof(size) || size == 0 ||
      size > 4096) {
    printf("[INFO] Unknown request\n");
    goto end;
  }

  struct Image *img = new_img(size);
  if (read(fd, img->data, size) != size) {
    printf("[INFO] Unknown request\n");
    goto end;
  }

  struct Hash h = make_hash(img->len, img->data + sizeof(img->len));
  add(&h, img);
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
  const size_t BUF_SIZE = ECC_BYTES;
  char buf[BUF_SIZE];
  struct sockaddr_in addr;
  size_t addr_len;
  char flag[FLAG_SIZE];

  while ((ptr->flags & 1) == 0) {
    ssize_t n = recvfrom(ptr->udp_fd, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&addr, &addr_len);
    if (n < 0) {
      printf("[ERROR] at recvfrom");
      continue;
    }

    if (n != BUF_SIZE) {
      printf("[WARN] received less data, than assumed: %d\n", n);
      continue;
    }

    struct Image *img = search((struct Hash *)buf);
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

void start(struct AbobCloudServer *server) {
  pthread_create(&server->img_serv, NULL, &img_server_main_loop, server);
  pthread_create(&server->flag_serv, NULL, &flag_server_main_loop, server);

  pthread_detach(server->img_serv);
  pthread_detach(server->flag_serv);
}
