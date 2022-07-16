/*<==============================================================================>*/
/*                                   EVENT_LOOP                                   */
/*                              ECHO_SERVER_on_LIBEV                              */
/*<==============================================================================>*/

/*
 * созададим delfault loop
 * struct ev_loop *loop = ev_default_loop(0);
 * удалить его можно ev_dafault_destroy();
 * нужно обратить внимание, что нет никаких параметров - это потому что
 * default'ный loop всегда один!
 *
 * можно создать НЕдефолтный loop
 * struct ev_loop *nd_loop = ev_loop_new(unsigned int flags);
 * удалить: ev_loop_destroy(nd_loop);
 *
 * Как узнать - текущий луп default'ный или нет?
 * int ev_is_default_loop(loop);
 *
 * Счетчик событий:
 * uint ev_loop_count(loop);
 *
 * Как папустить и остановить loop:
 * ev_loop(loop, flags);
 *    если флаг равен нулю - то loop будет выполняться бесконечно,
 *    пока не вызовем unloop;
 *    EVLOOP_ONESHOT - выполнится один раз;
 *    EVLOOP_NONBLOCK - неблокирующий режим.
 * ev_unloop(loop, how);
 *
 * Как работать с watcher'ами?
 * struct ev_loop *loop = ev_default_loop(0); // создали дефолтный loop
 * struct ev_io stdin_watcher; // создадим watcher, который будет слушать stdin
 * ev_init(stdin_watcher, my_cb); // инициализируем watcher;
 * ev_io_set(&stdin_watcher, STDIN_FILENO, EV_READ); // установим дескриптор stdin'а
 *                                                   // в этот watcher на чтение;
 * ev_io_start(loop, &stdin_watcher); // тем самым мы sdtin_watcher добавили в loop
 *                                    // и запустили watcher;
 * ev_loop(loop, 0); // теперь запускаем цикл.
 *
 * Теперь переходим к call_back'ам.
 * static void my_cb(*loop, *watcher, int revents) {
 *   ev_io_stop(watcher); // останавливаем watcher;
 *   ev_unloop(loop, EVUNLOOP_ALL); // выключаем loop.
 * }
 * // loop - это наш дефолтный луп;
 * // watcher - наш stdin_watcher
 * // revents - EV_READ.
 *
 * ***watcher'ы, которые реализуют функциональность таймеров***
 * struct ev_timer t;
 * ev_timer_init(&t, my_cb, 60, 0);
 *                ^    ^    ^   ^
 *                |    |    a   b
 *          наш таймер |    |   |
 *             наш callback |   |
 *          через какое время   |
 *           таймер сработает   |
 *              через какое время
 *              таймер повторится
 * Если b = 0
 *
 *     отмеряем a  |  и все...
 * |---------------|---------------------->
 *                 |
 *                 ^
 *        вызывается таймер
 *
 * Если b > 0
 *
 *     отмеряем a  |  b    b    b ... и так далее, пока сами не остановим.
 * |---------------|----|----|----|---------->
 *                 |    ^    ^    ^
 *                 ^  вызываем таймер
 *        вызывается таймер
 *
 * В нашем случае мы ждем 60 сек. и один раз вызываем наш callback
 *
 * ev_timer_start(loop, *t);
 *
 * Опишем callback
 * static void my_cb(*loop, *watcher, revents) { // watcher - наш таймер!
 *   // ...
 * }
 *
 * ***Работа libev с сигналами***
 *
 * struct ev_signal w;
 * ev_signal_init(&w, my_cb, SIGINT);
 * ev_signal_start(loop, &w);
 *
 * а my_cb будет таким:
 * static void *my_cb(loop, w, revents) {
 *   // ...
 * }
 *
 *
 * ***Еще мы можем принимать т.н. сигналы SIGCHLD***
 * То есть принимать сигналы о том, что наши потомки процессы завершились
 * if (pid == fork()) {
 *   ev_child w;
 *   ev_child_init(&w, my_cb, pid, 0);
 *   ev_chid_start(...);       ^
 * }                           |---------- pid нашего дочернего процесса(потомка)
 *
 *
 * ***wather, который следит за статистикой файлов***
 * watcher привязывается к файлу и срабатывает тогда, когда с файлом что-то
 * сделали, например, что-то в него записали. То есть watcher мониторит
 * файловую систему.
 *
 * ev_start watcher;
 * ev_start_init(&watcher, my_cb, "/etc/passwd", 0);
 * se_start(*loop, &watcher);
 *
 * У watcher'а есть некоторые поля в структуре, которые можно
 * смотреть, например, в callback'е:
 *
 * watcher -> attr.st_size;
 * watcher -> attr.st_mtime;
 * watcher -> attr.st_atime;
 *
 * ***watcher, выполняющийся тогда, когда нам нечего делать***
 *                       ***EV_IDLE***
 * ev_idle w;
 * ev_idle_init(&w, my_cb);
 * ev_idle_start(loop, &w);
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ev.h>

void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  // создаем буфер
  char buffer[1024];

  // читаем
  // чтобы работало на windows меняем ssize_t на int
  ssize_t read_count = recv(watcher->fd, buffer, 1024, MSG_NOSIGNAL);
                              // MSG_NOSIGNAL - если соединение закрыто,
                              // не генерировать сигнал SIG_PIPE
  // обрабатываем ошибку
  if (read_count < 0) { return; }

  if (read_count == 0) { // соединение закрылось
    ev_io_stop(loop, watcher); // останавливаем watcher
    free(watcher); // высвобождаем память
    return;
  } else { // иначе передаем
    send(watcher->fd, buffer, read_count, MSG_NOSIGNAL);
  }
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  // принимаем соединение
  int client_socket = accept(watcher->fd, NULL, 0);
                                        // NULL - struct sockaddr *
                                        // то есть ip-адрес и порт клиента
                                        // 0 - размер этой структуры
                                        // если указать NULL, 0 - игнорируем
                                        // информацию о клиенте
  // watcher на чтение
  // выделяем память чтобы при выходе из call_back
  // область видимости структуры сохранилась в КУЧЕ
  struct ev_io *w_client = (struct ev_io*) malloc(sizeof(struct ev_io));
  ev_io_init (w_client, read_cb, client_socket, EV_READ);
  ev_io_start(loop, w_client);
}

int main() {
  struct ev_loop *loop = ev_default_loop(0);

  int sd;
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket_err!");
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sd, (struct sockaddr *)(&addr), sizeof(addr)) < 0) {
    printf("bind_err!");
    return -2;
  }

  listen(sd, SOMAXCONN);

  struct ev_io w_accept;
  ev_io_init(&w_accept, accept_cb, sd, EV_READ);
  ev_io_start(loop, &w_accept);

  while (1) {
    ev_loop(loop, 0);
  }

  return 0;
}
