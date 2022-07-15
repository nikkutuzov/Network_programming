/*<==============================================================================>*/
/*                                  TCP_Server                                    */
/*                            Elementary_Echo_Server                              */
/*<==============================================================================>*/
#include<stdio.h>

#include <sys/types.h> // для сокета
#include <sys/socket.h> // для сокета
#include <netinet/in.h> // для констант IPPROTO INADDR
#include <unistd.h> // для close();
#include <strings.h> // для bzero();

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
  int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // проверяем
  if (MasterSocket < 0) {
    printf("Socket_err!");
    return -1;
  }

  // структура адрес
  struct sockaddr_in SockAddr;

  // занулим (необязательно!)
  bzero(&SockAddr, sizeof(SockAddr));

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
  // sockfd - дискриптор сокета, который мы хотим привязать к заданному адресу;
  // addr - указатель на структуру с адресом(см. выше);
  // addrlen - размер этой структуры.
  if ((bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr))) < 0) {
      printf("bind_err");
      return -2;
  }

  // открываем слушающий TCP сокет
  listen(MasterSocket, SOMAXCONN);  // длина очереди ожидания: 128 - для linux < 5.4
                                    //                       4096 - для linux >= 5.4
  while(1) {
    // создаем и проверяем accept
    // прототип int accept(int sockfd, void *addr, int *addrlen);
    // sockfd - задаем слушающий сокет7 После вызова он остается в слушающем
    // состоянии и может принимать другие соединения;
    // addr - в структуру, на которую ссылается addr, записывается адрес сокета
    // клиента, который установил соединение с сервером;
    // addrlen - записывается размер структуры. Функция accept записывает туда
    // длину, которая реально была использована;
    //
    // возвращаемое значение - новый(клиентский) сокет.
    int SlaveSocket = accept(MasterSocket, NULL, 0);
      // NULL - struct sockaddr *
      // то есть ip-адрес и порт клиента;
      // 0 - размер этой структуры;
      // если указать NULL, 0 - игнорируем
      // информацию о клиенте.
    if (SlaveSocket < 0) {
      printf("accept_err");
      return -3;
    }

    while(1) {
      int buffer[1024]; // создадим буфер

      // в него прочитаем переданные серверу данные
      int recv_counter = recv(SlaveSocket, buffer, 1024, MSG_NOSIGNAL);
        // MSG_OOB - предписывает отправить данные как срочные;
        // MSG_DONTROUTE - запрещает маршрутизацию пакетов. "Нижележащие"
        // транспортные слои могут проигнорировать этот флаг;
        // MSG_NOSIGNAL - если соединение закрыто, не генерировать сигнал SIG_PIPE;
        // если флаги не используются - 0.

        // если данных нет(0) или они говорят об ошибке(<0)
      if (recv_counter <= 0) { break; } // прерываемся
        // если все хорошо, отправим прочитанные (не всегда весь буфер) обратно

      send(SlaveSocket, buffer, recv_counter, MSG_NOSIGNAL);
        // отправим клиенту данные из буфера.
    }

    // прототип int shutdown(int sockfd, int how);
    // sockfd - сокет;
    // how - флаг.
    shutdown(SlaveSocket, SHUT_RDWR); // разрываем соединение - запрещаем передачу
                                      // данных, флаги:
                                      // SHUT_RD или 0 - запрещаем чтение из сокета;
                                      // SHUT_WR или 1 - запрещаем запись в сокет;
                                      // SHUT_RDWR или 2 - запрещаем и то и другое.
    close(SlaveSocket); // закрываем сокет
      // если закрыть сокет до разрыва соединения, то сокет закроется,
      // а соединение НЕ разорвется!
  }

  return 0;
}

