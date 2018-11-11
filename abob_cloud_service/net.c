#include "net.h"
#include "common.h"
#include "crypto.h"
#include "img_db.h"
#include "img_cache.h"
#include "net.h"

#define __USE_MISC

#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

static int create_socket_common(short port, int type) {
  struct sockaddr_in serv_addr;

  int sockfd = socket(AF_INET, type, 0);
  if (sockfd < 0) {
    die("at opening socket");
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    die("setsockopt failed");
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

  if (listen(fd, 0) != 0) {
    die("at listening");
  }

  return fd;
}

static int create_udp(short port) {
  return create_socket_common(port, SOCK_DGRAM);
}

FILE *hashes_logger;

static inline void handle_upload(int fd) {
  size_t size;
  size_t sz_read = read(fd, &size, sizeof(size));
  if (sz_read != sizeof(size) || size == 0 || size > MAX_IMG) {
    printf("[INFO] Unknown request, read: %d, max image size is %d\n", sz_read, MAX_IMG);
    goto end;
  }

  struct Image *img = new_img(size);
  sz_read = 0;
  size_t cur_sz;
  for (cur_sz = 0;
    sz_read < size &&
      (cur_sz = read(fd, img->data + cur_sz, size - sz_read)) > 0;
    sz_read += cur_sz);

  if (sz_read != size) {
    printf("[INFO] Unknown request, read: %d bytes instead of %d, []\n", sz_read, size);
    if (cur_sz < 0) {
      print("[WARN] read error [%d]: %s\n" , cur_sz, strerror(errno));
    }
  } else {
    if (hashes_logger != NULL) {
      fprintf(hashes_logger, img->data, "%s");
    }
    struct Hash h = make_hash(img->data, img->len);
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
    if (new_fd < 0) {
      die("at accept");
    }
    int pid = fork();
    if (pid < 0) {
      die("at fork");
    }
    if (pid == 0) {
      handle_upload(new_fd);
    }
  }
  pthread_exit(v);
}

static inline int is_hash(char *buf, size_t len) {
  int res = 1;
  for (size_t i = 0; i < len && res; ++i)
    res &= isgraph(buf[i]) && !isupper(buf[i]);
  return res;
}

static void *flag_server_main_loop(void *v) {
  struct AbobCloudServer *ptr = v;
  struct Hash hash;
  struct sockaddr_in addr = {};
  size_t addr_len = sizeof(addr);

  while ((ptr->flags & 1) == 0) {
    ssize_t n = recvfrom(ptr->udp_fd, hash.data, HEX_HASH_SIZE, 0,
                         (struct sockaddr *)&addr, &addr_len);
    if (n < 0) {
      printf("[ERROR] at recvfrom %s\n", strerror(errno));
      continue;
    }

    if (n != HEX_HASH_SIZE) {
      printf("[WARN] received less data, than assumed: %d instead of %d\n", n, HEX_HASH_SIZE);
      continue;
    }

    if (!is_hash(hash.data, HEX_HASH_SIZE)) {
      printf("[WARN] received corrupted hash\n");
      continue;
    }

    printf("[INFO] received hash: %s\n", hash.data);
    struct Image *img = search(&hash);
    if (img == NULL) {
      printf("[WARN] image isn't found, skipping\n");
      continue;
    }

    ssize_t res = sendto(ptr->udp_fd, img->flag, FLAG_SIZE, 0, (struct sockaddr *)&addr,
           addr_len);
    if (res < 0) {
      printf("[WARN] sendto errno: %d, %s\n", ptr->udp_fd, strerror(errno));
      char ip_str[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
      printf("Host: %s:%hd\n", ip_str, addr.sin_port);
    }
    search_destruct(img);
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
  if (hashes_logger != NULL) {
    fclose(hashes_logger);
  }
}

void start(struct AbobCloudServer *server) {
  hashes_logger = fopen("/tmp/log_hashes", "wb");

  if (hashes_logger == NULL) {
    printf("[WARN] cannot open file for logging hashes, no logs would be "
           "written\n");
  }

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
