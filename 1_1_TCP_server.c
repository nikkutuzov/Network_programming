/*<==============================================================================>*/
/*                                  TCP_Server                                    */
/*                            Elementary_Echo_Server                              */
/*<==============================================================================>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO INADDR
#include <unistd.h> // для close();
#include <strings.h> // для bzero();
#include <arpa/inet.h> // для IP

int Socket(int domain, int type, int protocol);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void Shutdown(int sockfd, int how);
void Close(int fd);

int main() {
  // создаем сокет:
  int MasterSocket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // структура адрес
  struct sockaddr_in SockAddr;

  // занулим (необязательно!)
  bzero(&SockAddr, sizeof SockAddr);

  // заполним
  SockAddr.sin_family = AF_INET; // Internet-домен
  SockAddr.sin_port = htons(12345); // номер порта (htons - приводим
                                    // число с сетевому проядку байт short)
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); // IP-адрес хоста (htonl - приводим
                                            // число к сетевому порядку байт long)
                                            // INADDR_ANY              0.0.0.0
                                            // INADDR_LOOPBACK       127.0.0.1
  // чтобы указать конкретный ip-адрес используется ip = inet_addr("10.0.0.1");
  // или новая ip = inet_pton(AF_INET, "10.0.0.1", &(address.sin_addr);

  // связываем сокет и адрес:
  Bind(MasterSocket, (struct sockaddr *) &SockAddr, sizeof SockAddr);

  // открываем слушающий TCP сокет
  Listen(MasterSocket, SOMAXCONN);  // длина очереди ожидания: 128 - для linux < 5.4
                                    //                       4096 - для linux >= 5.4
  while(1) {
    // переменная, хранящая размер структуры SockAddr
    socklen_t addrlen = sizeof SockAddr;
    // accept
    int SlaveSocket = Accept(MasterSocket, (struct sockaddr *) &SockAddr, &addrlen);

    while(1) {
      char buffer[1024]; // создадим буфер

      // в него прочитаем переданные серверу данные
      ssize_t recv_count = recv(SlaveSocket, buffer, 1024, MSG_NOSIGNAL);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.


/*    *** Хорошо бы сделать вот так: мы проверяем и ошибку и отсутствие данных
      // ошибка
      if (recv_count == -1) {
        perror("recv_err!");
        exit(EXIT_FAILURE);
      }
      // данных нет
      if (recv_count == 0) {
        printf("SHUTDOWN FROM CLIENT");
        exit(EXIT_SUCCESS);
      }
      *** но сделаем просто:
*/

      if(recv_count <= 0) { break; }

      // если все хорошо, отправим прочитанные (не всегда весь буфер) обратно
      // прототип send(int sockfd, const void *msg, int len, int flags);
      // sockfd - сокет;
      // msg - сообщение;
      // len - длина сообщения;
      // flags - флаги.
      send(SlaveSocket, buffer, recv_count, MSG_NOSIGNAL);
      // MSG_OOB - предписывает отправить данные как срочные;
      // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
      // транспортные слои могут проигнорировать этот флаг;
      // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
      // если флаги не используются - 0.
    }

    Shutdown(SlaveSocket, SHUT_RDWR); // разрываем соединение - запрещаем передачу
    Close(SlaveSocket); // закрываем сокет
      // если закрыть сокет до разрыва соединения, то сокет закроется,
      // а соединение НЕ разорвется!
  }

  return 0;
}

int Socket(int domain, int type, int protocol) {
  // прототип int socket(int domain, int type, int protocol);
  // domain:
  //    Константа AF_INET - для Internet-домена (ip v.4);
  //    Константа AF_INET6 - для Internet-домена (ip v.6);
  //    Константа AF_UNIX - для передачи данных, используя файловую
  //    систему ввода-вывода UNIX(исползуется для межпроцессного
  //    взаимождействия на одном компьютере и не годятся для работы по сети).
  // type:
  //    SOCK_STREAM - для работы по протоколу TCP;
  //    SOCK_DGRAM - для работы по протоколу UDP (датаграммы);
  //    SOCK_RAW - для низкоуровневых("сырых") сокетов - IP, ICMP, etc.
  // protocol:
  //    0 - протокол по умолчанию, то есть для SOCK_STREAM - это будет TCP,
  //    для SOCK_DGRAM - UDP, но можно исползовать и следующие константы:
  //    IPPROTO_TCP и IPPROTO_UDP - то есть указать протокол явно.
  //
  // В качестве параметра сокет возвращает параметр типа int - дескриптор сокета:
  //    положительное число - сам дескриптор;
  //    -1 - ошибка.
int res = socket(domain, type, protocol);

  // проверяем
  if (res == -1) { // если ошибка
    perror("socket_err!"); // распечатываем не номер ошибки, а ее строковое значение
    exit(EXIT_FAILURE); // и выходим в этом случае
  }
  // елси ошибки нет, возвращаем дескриптор сокета
  return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  // прототип int bind(int sockfd, struct sockaddr *addr, int addrlen);
  // sockfd - дескриптор сокета, который мы хотим привязать к заданному адресу;
  // addr - указатель на структуру с адресом(см. выше);
  // addrlen - размер этой структуры.
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
  // прототип int accept(int sockfd, void *addr, int *addrlen);
  // sockfd - задаем слушающий сокет. После вызова он остается в слушающем
  // состоянии и может принимать другие соединения;
  // addr - в структуру, на которую ссылается addr, записывается адрес сокета
  // клиента, который установил соединение с сервером;
  // addrlen - записывается размер структуры. Функция accept записывает туда
  // длину, которая реально была использована;
  //
  // возвращаемое значение - новый(клиентский) сокет.
  // если указать NULL, 0 - игнорируем
  // информацию о клиенте.
  int res = accept(sockfd, addr, addrlen);
  if (res == -1) {
    perror("accept_err!");
    exit(EXIT_FAILURE);
  }
  return res;
}

void Shutdown(int sockfd, int how) {
  // прототип int shutdown(int sockfd, int how);
  // sockfd - сокет;
  // how - флаг.
  // флаги:
  // SHUT_RD или 0 - запрещаем чтение из сокета;
  // SHUT_WR или 1 - запрещаем запись в сокет;
  // SHUT_RDWR или 2 - запрещаем и то и другое.
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
