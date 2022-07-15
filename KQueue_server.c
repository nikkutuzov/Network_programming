/*<==============================================================================>*/
/*                                  EPOLL_SERVER                                  */
/*<==============================================================================>*/
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/event.h>

int SetNonBlock(int fd) {
  int flags;
#if defined(O_NONBLOCK)
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
    flags = 0;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif
}

int main() {
  int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (MasterSocket < 0) { printf("socket_err"); return -1; }

  struct sockaddr_in SockAddr;
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

   if (bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)) < 0) {
    printf("bind_err");
    return -2;
  }

  SetNonBlock(MasterSocket);

  listen(MasterSocket, SOMAXCONN);

  // создаем дескриптор kqueue
  int KQueue = kqueue();

  struct kevent KEvent;
  bzero(&KEvent, sizeof(KEvent));

  EV_SET(&KEvent, MasterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
  kevent(KQueue, &KEvent, 1, NULL, 0, NULL);

  while(1) {
    bzero(&KEvent, sizeof(KEvent));
    kevent(KQueue, NULL, 0, &KEvent, 1, NULL);

    if (KEvent.filter == EVFILT_READ) {
      if (KEvent.ident == MasterSocket) {
        int SlaveSocket = accept(MasterSocket, NULL, 0);
        if (SlaveSocket < 0) { printf("accept_err"); return -3; }
        SetNonBlock(SlaveSocket);

        bzero(&KEvent, sizeof(KEvent));
        EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
        kevent(KQueue, &KEvent, 1, NULL, 0, NULL);
      } else {
        static char buffer[1024];

        int recv_count = recv(KEvent.ident, buffer, 1024, MSG_NOSIGNAL);
        if (recv_count <= 0) {
          close(KEvent.ident);
          printf("disconnected!\n");
        } else {
          send(KEvent.ident, buffer, recv_count, MSG_NOSIGNAL);
        }
      }
    }
  }

  return 0;
}
