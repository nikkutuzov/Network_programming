/*<==============================================================================>*/
/*                                 UDP_Receiver                                   */
/*<==============================================================================>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO INADDR
#include <strings.h> // для bzero();
#include <arpa/inet.h> // для IP

int Socket(int domain, int type, int protocol);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int main() {
  // счетчик прочитанных данных
  int recv_count = 0;
  // буфер для данных
  char buffer[1024];

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

  // связываем сокет и адрес:
  // прототип int bind(int sockfd, struct sockaddr *addr, int addrlen);
  // sockfd - дескриптор сокета, который мы хотим привязать к заданному адресу;
  // addr - указатель на структуру с адресом(см. выше);
  // addrlen - размер этой структуры.
  Bind(Sock, (struct sockaddr *) &SockAddr , sizeof SockAddr);

  while(1) {
    // прототип recvfrom(int sockfd, void *buf, int len, unsigned int flags,
    //                   struct sockaddr *from, int *fromlen);
    // sockfd - сокет;
    // buf - буфер, куда поместим полученные данные;
    // len - размер полученных данных;
    // flags - флаги;
    // from - адрес отправителя;
    // fromlen - длина адреса отправителя.
    recv_count = recvfrom(Sock, buffer, 1024, 0, NULL, NULL);

    // установим в буфере конец полученного сообщения
    buffer[recv_count] = '\0';

    // распечатаем полученное сообщение
    printf(buffer);
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
