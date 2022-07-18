/*<==============================================================================>*/
/*                                  TCP_Client                                    */
/*                            Elementary_Echo_Client                              */
/*                             NB: 1_1_TCP_server.c                               */
/*<==============================================================================>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>

int Socket(int domain, int type, int protocol) {
  int res = socket(domain, type, protocol);

  if (res == -1) {
    perror("socket_err!");
    exit(EXIT_FAILURE);
  }

  return res;
}

void Connect(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
  int res = connect(sockfd, addr, addrlen);

  if (res == -1) {
    perror("connect_err!");
    exit(EXIT_FAILURE);
  }
}

void Shutdown(int sockfd, int how) {
  int res = shutdown(sockfd, how);
  if (res == -1) {
    perror("shutdown_err!");
    exit(EXIT_FAILURE);
  }
}

void Close(int fd) {
  int res = close(fd);
  if (res == -1) {
    perror("close_err!");
    exit(EXIT_FAILURE);
  }
}

int main() {
  int SlaveSocket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in SockAddr;
  bzero(&SockAddr, sizeof SockAddr);
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  Connect(SlaveSocket, (struct sockaddr *) &SockAddr, sizeof SockAddr);

  // создаем буфер с данными
  char buffer[] = "BANG";

  // отправим серверу
  send(SlaveSocket, buffer, 4, MSG_NOSIGNAL);

  // получим обратно от сервера
  // прототип recv(int sockfd, void *buf, int len, int flags);
  // sockfd - сокет;
  // buf - буфер для сообщения;
  // len - длина сообщения;
  // flags - флаги.
  recv(SlaveSocket, buffer, 4, MSG_NOSIGNAL);

  printf(buffer);

  Shutdown(SlaveSocket, SHUT_RDWR);
  Close(SlaveSocket);

  return 0;
}
