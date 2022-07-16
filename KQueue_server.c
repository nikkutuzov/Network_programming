/*<==============================================================================>*/
/*                                  EPOLL_SERVER                                  */
/*                            for FreeBSD or macOS ONLY!                          */
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

  // создадим событие
  struct kevent KEvent; // <--- может быть не один, а, например, 10 <---
  bzero(&KEvent, sizeof(KEvent));                             //        |
                                                              //        |
  // заполним поля события с помощью макроса                            |
  // &KEvent - сслыка на структуру событие                              |
  // MasterSocket - сокет                                               |
  // EVFILT_READ - какие события будем отслеживать: готовность к чтению |
  // EV_ADD - добавим фильтр на чтение (EVFILT_READ);                   |
  EV_SET(&KEvent, MasterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);//        |
                                                              //        |
  // KQueue - дескриптор kqueue;                                        |
  // &KEvent - ссылка на событие;                                       |
  // 1 - сколько событий будем добавлять NB: можно хоть 10 за раз!      |
  // KEvent - массив из одного элемента <--------------------------------
  // первый NULL и следующий за ним 0 - возвращаемые события, в данном примере
  // ничего не возвращаем, но имеем право запросить все текущие на данный момент
  // события;
  // последний NULL - таймаут.
  kevent(KQueue, &KEvent, 1, NULL, 0, NULL);

  while(1) {
    bzero(&KEvent, sizeof(KEvent));
    // kevent для получения событий:
    // KQueue - дескритпор kqueue;
    // NULL и 0 - см. выше - регистрируем 0 событий;
    // KEvent - куда поместить;
    // 1 - сколько получаем событий;
    // NULL - таймаут.
    kevent(KQueue, NULL, 0, &KEvent, 1, NULL);

    // пришло ли событие на чтение?
    if (KEvent.filter == EVFILT_READ) {
      // если это MasterSocket
      if (KEvent.ident == MasterSocket) {
        // принимаем соединение
        int SlaveSocket = accept(MasterSocket, NULL, 0);
        if (SlaveSocket < 0) { printf("accept_err"); return -3; }
        SetNonBlock(SlaveSocket);

        bzero(&KEvent, sizeof(KEvent));
        EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
        kevent(KQueue, &KEvent, 1, NULL, 0, NULL);
      } else { // иначе - в этом случае пробуем читать
        static char buffer[1024];

        int recv_count = recv(KEvent.ident, buffer, 1024, MSG_NOSIGNAL);
        if (recv_count <= 0) { // если < 0 - на самом деле - ошибка,
                               // 0 - соеденение закрылось
          close(KEvent.ident);
          printf("disconnected!\n");
        } else { // иначе отправляем то, что прочитали
          send(KEvent.ident, buffer, recv_count, MSG_NOSIGNAL);
        }
      }
    }
  }

  return 0;
}
