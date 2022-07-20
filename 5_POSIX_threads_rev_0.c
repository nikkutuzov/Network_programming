/*<==============================================================================>*/
/*                             POSIX_THREADS - потоки                             */
/*<==============================================================================>*/

/*
 * В одном потосе одни сокет(свой) и свой event_loop, порт 12345
 * Во втором потоке тоже свой сокет и свой event_loop, порт 54321
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ev.h>

#include <pthread.h>

void *MyThreadFun(void *arg);

int Socket(int domain, int type, int protocol);
void Bind(int sockfd, const struct sockaddr *addr,
         socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void Send(int sockfd, const void *buf, size_t len, int flags);
ssize_t Recv(int sockfd, void *buf, size_t len, int flags);
void Shutdown(int sockfd, int how);
void Close(int fd);

void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

int main() {
  struct ev_loop *loop_pthr_1 = ev_loop_new(0);

  int MasterSocket_pthr_1 = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  pthread_t thr;
  pthread_create(&thr, NULL, MyThreadFun, NULL);


  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  // IP после зануления структуры будет 0.0.0.0 - можно явно не указывать

  Bind(MasterSocket_pthr_1, (struct sockaddr *) &addr, sizeof addr);

  Listen(MasterSocket_pthr_1, 5);

  struct ev_io w_accept;
  ev_io_init(&w_accept, accept_cb, MasterSocket_pthr_1, EV_READ);
  ev_io_start(loop_pthr_1, &w_accept);

  while(1) {
    ev_loop(loop_pthr_1, 0);
  }

  pthread_join(thr, NULL);

  return 0;
}

void *MyThreadFun(void *args) {
  struct ev_loop *loop_pthr_2 = ev_loop_new(0);

  int MasterSocket_pthr_2 = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(54321);

  Bind(MasterSocket_pthr_2, (struct sockaddr *) &addr, sizeof addr);

  Listen(MasterSocket_pthr_2, 5);

  struct ev_io w_accept;
  ev_io_init(&w_accept, accept_cb, MasterSocket_pthr_2, EV_READ);
  ev_io_start(loop_pthr_2, &w_accept);

  while(1) {
    ev_loop(loop_pthr_2, 0);
  }

  return NULL;
}

int Socket(int domain, int type, int protocol) {
  int res = socket(domain, type, protocol);

  if (res == -1) {
    perror("socket_error!");
    exit(EXIT_FAILURE);
  }

  return res;
}

void Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
  int res = bind(sockfd, addr, addrlen);
  if (res == -1) {
    perror("bind_error!");
    exit(EXIT_FAILURE);
  }
}

void Listen(int sockfd, int backlog) {
  int res = listen(sockfd, backlog);
  if (res == -1) {
    perror("listen_error!");
    exit(EXIT_FAILURE);
  }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  int res = accept(sockfd, addr, addrlen);
  if (res == -1) {
    perror("accept_error!");
    exit(EXIT_FAILURE);
  }

  return res;
}

void Send(int sockfd, const void *buf, size_t len, int flags) {
  int res = send(sockfd, buf, len, flags);
  if (res == -1) {
    perror("send_error!");
    exit(EXIT_FAILURE);
  }
}

ssize_t Recv(int sockfd, void *buf, size_t len, int flags) {
  int res = recv(sockfd, buf, len, flags);
  if (res == -1) {
    perror("recv_error!");
    exit(EXIT_FAILURE);
  }

  return res;
}

void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  char buffer[1024];
  ssize_t read_count = Recv(watcher->fd, buffer, 1024, MSG_NOSIGNAL);

  if (read_count == 0) {
    ev_io_stop(loop, watcher);
    Shutdown(watcher->fd, SHUT_RDWR);
    Close(watcher->fd);
    free(watcher);
    return;
  } else {

    Send(watcher->fd, buffer, read_count, MSG_NOSIGNAL);
  }
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  int client_socket = Accept(watcher->fd, NULL, 0);
  struct ev_io *w_client = (struct ev_io*) malloc(sizeof(struct ev_io));

  ev_io_init (w_client, read_cb, client_socket, EV_READ);
  ev_io_start(loop, w_client);
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
