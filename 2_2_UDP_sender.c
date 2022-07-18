/*<==============================================================================>*/
/*                                  UDP_Sender                                    */
/*<==============================================================================>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO INADDR
#include <unistd.h> // для close();
#include <strings.h> // для bzero();
#include <arpa/inet.h> // для IP

// сообщения:
char message_1[] = "Hello there!\n";
char message_2[] = "Bye Bye!\n";

int Socket(int domain, int type, int protocol);
void Connect(int sockfd, struct sockaddr *addr, socklen_t addrlen);
void Close(int fd);

int main() {
  // создаем сокет:
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
  int Sock = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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

  // ДЕМОНСТРАЦИЯ ОБЫЧНОГО СОКЕТА
  // прототип int sendto(int sockfd, const void *msg, int len, unsigned int flags,
  //                     const struct sockaddr *to, int tolen);
  // sockfd - сокет;
  // msg - что мы будем отправлять
  // len - длина отправляемого сообщения;
  // flags - флаги;
  // to - адрес получателя;
  // tolen - длина адреса получателя.
  sendto(Sock, message_1, sizeof message_1 , 0,
         (struct sockaddr *) &SockAddr, sizeof SockAddr);

  // ДЕМОНСТРАЦИЯ ПРИСОЕДИНЕННОГО СОКЕТА
  // Некоторую путаницу вносят присоединённые датаграммные сокеты
  // (connected datagram sockets). Дело в том, что для сокета с типом SOCK_DGRAM
  // тоже можно вызвать функцию connect, а затем использовать send и recv для обмена
  // данными. Нужно понимать, что никакого соединения при этом не устанавливается.
  // Операционная система просто запоминает адрес, который вы передали функции
  // connect, а затем использует его при отправке данных. Обратите внимание, что
  // присоединённый сокет может получать данные только от сокета, с которым он
  // соединён.
  //
  // пытаемся соединиться и проверяем
  Connect(Sock, (struct sockaddr *) &SockAddr, sizeof SockAddr);

  // отправим серверу
  // прототип send(int sockfd, const void *msg, int len, int flags);
  // sockfd - сокет;
  // msg - сообщение;
  // len - длина сообщения;
  // flags - флаги.
  send(Sock, message_2, sizeof message_2, MSG_NOSIGNAL);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.

  // закрываем сокет
  Close(Sock);

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

void Close(int fd) {
  int res = close(fd);
  if (res == -1) {
    perror("close_err!");
    exit(EXIT_FAILURE);
  }
}
