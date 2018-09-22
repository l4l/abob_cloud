#include "net.h"
#include "common.h"

#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

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

int create_tcp(short port) {
  int fd = create_socket_common(port, SOCK_STREAM);

  if (listen(fd, 5) != 0) {
    die("at listening");
  }

  return fd;
}

int create_udp(short port) { return create_socket_common(port, SOCK_DGRAM); }
