/*<==============================================================================>*/
/*                                  UDP_Sender                                    */
/*<==============================================================================>*/
#include <stdio.h>

#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO INADDR
#include <unistd.h> // для close();

// сообщения:
char message_1[] = "Hello there!\n";
char message_2[] = "Bye Bye!\n";

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
  // В качестве параметра сокет возвращает параметр типа int - дискриптор сокета:
  //    положительное число - сам дискриптор;
  //    -1 - ошибка.
  int Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // проверяем
  if (Socket < 0) {
    printf("Socket_err!");
    return -1;
  }

  // структура адрес
  struct sockaddr_in SockAddr;

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

  // прототип int sendto(int sockfd, const void *msg, int len, unsigned int flags,
  //                     const struct sockaddr *to, int tolen);
  // sockfd - сокет;
  // msg - что мы будем отправлять
  // len - длина отправляемого сообщения;
  // flags - флаги;
  // to - адрес получателя;
  // tolen - длина адреса получателя.
  sendto(Socket, message_1, sizeof(message_1), 0,
         (struct sockaddr *)(&SockAddr), sizeof(SockAddr));

  // пытаемся соединиться и проверяем
  connect(Socket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));
  if (connect(Socket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)) < 0) {
    printf("connect_err");
    return -2;
  }

  // отправим серверу
  // прототип send(int sockfd, const void *msg, int len, int flags);
  // sockfd - сокет;
  // msg - сообщение;
  // len - длина сообщения;
  // flags - флаги.
  send(Socket, message_2, sizeof(message_2), MSG_NOSIGNAL);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.

  // закрываем сокет
  close(Socket);

  return 0;
}
