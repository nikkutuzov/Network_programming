/*<==============================================================================>*/
/*                                  TCP_Client                                    */
/*                            Elementary_Echo_Client                              */
/*<==============================================================================>*/
#include <stdio.h>

#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO_... INADDR_...
#include <unistd.h> // для close();

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
  int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

  // пытаемся соединиться и проверяем
  // прототип connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
  // sockfd - сокет, который будет исползоваться для обмена данными с сервером;
  // serv_addr - указатель на структуру с адресом сервера;
  // addrlen - длина адреса.
  if (connect(Socket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)) < 0) {
    printf("connect_err");
    return -2;
  }

  // создаем буфер с данными
  char buffer[] = "BANG";

  // отправим серверу
  // прототип send(int sockfd, const void *msg, int len, int flags);
  // sockfd - сокет;
  // msg - сообщение;
  // len - длина сообщения;
  // flags - флаги.
  send(Socket, buffer, 4, MSG_NOSIGNAL);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.

  // получим обратно от сервера
  // прототип recv(int sockfd, void *buf, int len, int flags);
  // sockfd - сокет;
  // buf - буфер для сообщения;
  // len - длина сообщения;
  // flags - флаги.
  recv(Socket, buffer, 4, MSG_NOSIGNAL);

  // распечатаем то, что получили
  printf(buffer);

  // прототип int shutdown(int sockfd, int how);
  // sockfd - сокет;
  // how - флаг.
  shutdown(Socket, SHUT_RDWR); // разрываем соединение - запрещаем передачу
                                 // данных, флаги:
                                 // SHUT_RD или 0 - запрещаем чтение из сокета;
                                 // SHUT_WR или 1 - запрещаем запись в сокет;
                                 // SHUT_RDWR или 2 - запрещаем и то и другое.
  close(Socket); // закрываем сокет
                 // если закрыть сокет до разрыва соединения, то сокет закроется,
                 // а соединение НЕ разорвется!

  return 0;
}
