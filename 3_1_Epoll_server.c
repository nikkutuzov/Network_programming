/*<==============================================================================>*/
/*                                  EPOLL_SERVER                                  */
/*<==============================================================================>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <strings.h>
#include <arpa/inet.h>

#define MAX_EVENTS 32 // максимальное количество событий за раз

/*<-----функция_делает_сокет_неблокирующим----->*/
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

int Socket(int domain, int type, int protocol);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void Send(int sockfd, const void *buf, size_t len, int flags);
void Shutdown(int sockfd, int how);
void Close(int fd);

int main() {
  int MasterSocket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in SockAddr;
  bzero(&SockAddr, sizeof SockAddr);
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(MasterSocket, (struct sockaddr *) &SockAddr, sizeof SockAddr);

  // делаем MasterSocket неблокирующим
  SetNonBlock(MasterSocket);

  Listen(MasterSocket, SOMAXCONN);

  // создаем дескриптор ePoll'а
  // прототип int epoll_create1(int flags);
  // flags - флаги - почти всегда 0;
  // Функция возвращает файловый дескриптор, который в дальнейшем
  // передаётся во все остальные вызовы epoll API.
  int EPoll = epoll_create1(0);

  // зарегистрируем MasterSocket в этом EPoll'е
  struct epoll_event Event;
  Event.data.fd = MasterSocket;
  Event.events = EPOLLIN; // какие события будем отслеживать:
              // EPOLLIN - новые данные (для чтения) в файловом
              //           дескрипторе - level-triggered - файловый дескриптор
              //           возвращается, если остались непрочитанные/записанные
              //           данные;
              // EPOLLOUT - файловый дескриптор готов продолжить принимать
              //            данные (для записи);
              // EPOLLERR - в файловом дескрипторе произошла ошибка;
              // EPOLLHUP - закрытие файлового дескриптора;
              // EPOLLET - edge-triggered - файловый дескриптор с событием
              //           возвращается только если с момента последнего возврата
              //           произошли новые события (например, пришли новые данные);

  // прототип int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
  // epfd - дескриптор Epoll;
  // op - возможные варианты:
    // EPOLL_CTL_ADD - Зарегистрировать файловый дескриптор назначения fd в
    //                 экземпляре epoll, на который указывает файловый дескриптор
    //                 epfd, и связать событие event с внутренним файлом,
    //                 указывающим на fd;
    // EPOLL_CTL_MOD - Изменить событие event, связанное с файловым дескриптором
    //                 назначения fd;
    // EPOLL_CTL_DEL - Удалить (отменить регистрацию) файлового дескриптора
    //                 назначения fd из экземпляра epoll, на который указывает epfd.
    //                 Значение event игнорируется и может быть NULL.
  // fd - сокет;
  // event - событие.
  epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event); // регистрируем событие

  while (1) {
    struct epoll_event Events[MAX_EVENTS];

    // получаем события:
    // прототип int epoll_wait(int epfd, struct epoll_event *events,
    //                         int maxevents, int timeout);
    // epfd - дескриптор Epoll;
    // events - массив событий;
    // maxevents - размер массива;
    // timeout - (-1) - таймаут - ждем вечно.
    int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);
    // N - сколько конкретно в этот раз вернулось событий (<= maxevents).

    // пробегаемся по ВСЕМ событиям
    for (unsigned int i = 0; i < N; ++i) {
      // если у нас MasterSocket
      if (Events[i].data.fd == MasterSocket) {
        // делаем accept
        int SlaveSocket = Accept(MasterSocket, NULL, 0);

        // делаем его неблокирующим
        SetNonBlock(SlaveSocket);

        // зарегистрируем SlaveSocket в EPOll'е
        struct epoll_event Event;
        Event.data.fd = SlaveSocket;
        Event.events = EPOLLIN;

        epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);
      } else { // усли НЕ MasterSocket, попробуем его прочитать
        static char buffer[1024];
        int recv_counter = recv(Events[i].data.fd, buffer, 1024, MSG_NOSIGNAL);
        if ((recv_counter == 0) && (errno != EAGAIN)) { // если данных нет,
                                                        // но ресурс доступен

          Shutdown(Events[i].data.fd, SHUT_RDWR);
          Close(Events[i].data.fd);
        } else if (recv_counter > 0) { //если прочитать данные удалось
          Send(Events[i].data.fd, buffer, recv_counter, MSG_NOSIGNAL);
        }
      }
    }
  }

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

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int res = bind(sockfd, addr, addrlen);
  if (res == -1) {
    perror("bind_err!");
    exit(EXIT_FAILURE);
  }
}

void Listen(int sockfd, int backlog) {
  int res = listen(sockfd, backlog);
  if (res == -1) {
    perror("listen_err!");
    exit(EXIT_FAILURE);;
  }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  int res = accept(sockfd, addr, addrlen);
  if (res == -1) {
    perror("accept_err!");
    exit(EXIT_FAILURE);
  }
  return res;
}

void Send(int sockfd, const void *buf, size_t len, int flags) {
  // прототип ssize_t send(int sockfd, const void *msg, int len, int flags);
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
