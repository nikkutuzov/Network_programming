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

int Socket(int domain, int type, int protocol);
void Connect(int sockfd, struct sockaddr *addr, socklen_t addrlen);
ssize_t Send(int sockfd, const void *buf, size_t len, int flags);
ssize_t Recv(int sockfd, void *buf, size_t len, int flags);
void Shutdown(int sockfd, int how);
void Close(int fd);

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
  Send(SlaveSocket, buffer, 4, MSG_NOSIGNAL);

  // получим обратно от сервера
  Recv(SlaveSocket, buffer, 4, MSG_NOSIGNAL);

  printf(buffer);

  Shutdown(SlaveSocket, SHUT_RDWR);
  Close(SlaveSocket);

  return 0;
}

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

ssize_t Send(int sockfd, const void *buf, size_t len, int flags) {
  // прототип send(int sockfd, const void *msg, int len, int flags);
  // sockfd - сокет;
  // msg - сообщение;
  // len - длина сообщения;
  // flags - флаги.
  ssize_t res = send(sockfd, buf, len, flags);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.

  if (res == -1) {
    perror("send_err!");
    exit(EXIT_FAILURE);
  }
}

// чтобы работало на windows меняем ssize_t на int
ssize_t Recv(int sockfd, void *buf, size_t len, int flags) {
  // прототип recv(int sockfd, void *buf, int len, int flags);
  // sockfd - сокет;
  // buf - буфер для сообщения;
  // len - длина сообщения;
  // flags - флаги.
  ssize_t res = recv(sockfd, buf, len, flags);

  if (res == -1) {
    perror("socket_err!");
    exit(EXIT_FAILURE);
  }

  return res;
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
