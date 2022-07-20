/*<==============================================================================>*/
/*                             POSIX_THREADS - потоки                             */
/*<==============================================================================>*/

/*
 * #include<pthread.h>
 *
 * // создание потока:
 * int pthread_create(pthread_t *thread_id, const pthread_attr_t *attr,
 *                    void *(*start_fcn)(void *), void *arg);
 * // thread_id - идентификатор потока;
 * // attr - атрибуты (см. ниже);
 * // 3-й параметр - функция запуска, то есть та функция, которая
 *                   будет запущена так сказать параллельно в потоке;
 * // agr - аргумент функции, который будет передан в нее.
 *
 * Создадим функцию:
 *
 * void *thread_func(void *value) {
 *   int *int_value = (int *)value; // передаем указатель на целое число
 *   (*int_value)++; // увеличим это число на единицу
 *   return value; // и вернем это число(указатель)
 * }
 *
 * Существует два вида потоков:
 * JOINABLE - любой поток, если мы не укажем, что он DETACHED, является JOINABLE.
 *            После завершения, joinable-поток должен вернуть результат и он
 *            засыпает до тех пор, пока не будет вызвана основным потоком функция
 *            pthread_join. Как только функция будет вызвана, поток вернет
 *            результат и умрет - это ПЕРВЫЙ СЦЕНАРИЙ.
 *            ВТОРОЙ СЦЕНАРИЙ: это когда засыпает ПЕРВЫЙ поток и ждет результата
 *            от второго потока.
 *
 * int pthread_join(pthread_t thread, void **retval);
 *
 * DETACHED - работает таким образом, что поток перестает нуждаться в join'е.
 *            Результат, который возвращается - теряется. Но зато мы добавили
 *            поток и забыли про него.
 *
 * int pthread_detach(pthread_t thread);
 *
 * ВАЖНОЕ ДОПОЛНЕНИЕ: join-поток может стать detach-потоком, а наоборот - нет!
 *
 * Теперь рассмотрим второй параметр - атрибуты - attr. Рассмотрим только один:
 * каким сделаем поток joinable или detached.
 * Для атрибутов есть специальная структура:
 * pthread_attr_t *attr;
 * pthread_att_init; // создание структуры
 * pthread_att_destroy; // удаление структуры
 *
 * Можем установить статус:
 * int pthread_setdetachstate(pthread_attr_t *attr, int detachstate);
 *                                                  /           |
 *                                                 /            |
 *                                                /             |
 *                                     PTHREAD_CREATE_JOINABLE  |
 *                                              или   PTHREAD_CREATE_DETACHED
 *
 * Завершение потоков бывает: явное и неявное.
 * Неявное:
 * void *thread_func(void *arg) {
 *   // ...
 *   return NULL; <--- не важно что, может быть и какое-то значение.
 * }            \
 *               Implicid termination - это и есть НЕявное завершение потока
 *
 *
 * Явное:
 *   Есть два вида:
 *   1) Явный вызов внутри потока:
 *      void pthread_exit(void *retval); - Explicid termination
 *      Этот вызов может быть только из самого потока, извне - не работает;
 *   2) Явный вызов команды завершения из другого потока:
 *      int pthread_cancel(pthread_t thread);
 *      В этом случае мы не получаем возвращаемого значения(прерываем до return).
 *
 * Убить поток можно только во время какого-нибудь системного вызова, например,
 * read или waitpid.
 * Функции - НЕ являются точками завершения, которые начинаются на pthread_*
 * Что еще НЕ точки точки завершения, например: free, malloc, realloc, calloc.
 *
 * Точки завершения можно ставить "руками": void pthread_testcancel(void);
 *
 * Еще несколько полезных функций:
 *
 * int pthread_setcancelstate(int state, int *oldstate);
 * state - статус, который устанавливаем, oldstate - статус, который был
 * // PTHREAD_CANCEL_ENABLE - разрешаем cancel извне
 * // PTHREAD_CANCEL_DISABLE - запрещаем cancel и убить поток извне тогда
 *                             НЕ получится
 *
 * Мы имеем точки завершения, однако есть режим, когда точки завершения не нужны.
 * Очень похоже на kill9 - можно убиться в любой момент. ЭТОТ РЕЖИМ - ОПАСНЫЙ!!!
 * НЕ рекомендуется к использованию!
 *
 * int pthread_setcanceltype(int type, int *oldtype);
 * type - устанавливаемый тип, oldtype - тип, который был
 * // PTHREAD_CANCEL_DEFERRED - стандартный режим с точками завершения
 * // PTHREAD_CANCEL_ASYNCHRONOUS (!!!) - асинхронный, когда мы точку завершения
 *                                        НЕ ждем, а завершаем непосредственно!
 *
 * Если мы остановили поток cancel'ом, то поток нам не вернет никакой статус. Если
 * мы остановленый поток попытаемся за'join'ить:
 *
 * int pthread_join(pthread_t thread, void **retval);
 * то retval = PTHREAD_CANCELED - значит поток был заcancel'ен.
 *
 * ***Mutex***
 *
 * Представим ситуацию, что мы используем один std::map двумя потоками, std::map не
 * преспособлен для такого и мы просто можем испортить данные этот словарь. Одним
 * из способов решения будет блокировка - Mutex - способ синхронизации потоков.
 * Mutex - mutual exclusion - "взаимное исключение". Mutex - это как бинарный
 * семафор, то есть либо 0 либо 1 - флажок поднят или опущен.
 *
 * Объявление:
 *
 * pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
 *
 * или
 *
 * int pthread_mutex_init(pthread_mutex_t *mp, const pthread_mutex_attr_t *mattrp);
 *
 * уничтожим:
 *
 * int pthread_mutex_destroy(pthread_mutex_t *mp);
 *
 * int pthread_mutex_lock(pthread_mutex_t *mp); - блокировка;
 * int pthread_mutex_unlock(pthread_mutex_t *mp); - разблокировка;
 * int pthread_mutex_trylock(pthread_mutex_t *mp); - попытка блокировки - если не
 *                                                   получится - вернется ошибка.
 *
 * Вообще, усыплять поток накладно, поэтому, если нужно быстро ставить mutex, а
 * потом быстро его снимать, то это дорого обходится. Усыплять поток целесообразно
 * надолго, а на короткий промежуток времени - нет. Решение - Spin.
 *
 * int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
 *
 * int pthread_mutex_destroy(pthread_spinlock_t *lock);
 *
 * int pthread_spin_lock(pthread_spinlock_t *lock); - блокировка;
 * int pthread_mutex_unlock(pthread_spinlock_t *lock); - разблокировка;
 * int pthread_mutex_trylock(pthread_spinlock_t *lock); - попытка блокировки - если не
 *                                                   получится - вернется ошибка.
 *
 * Отличается от mutex тем, что поток в этом месте НЕ спит, а крутит бесконечный цикл.
 *
 * RW-блокировки
 *
 * int pthread_rwlock_init(pthread_rwlock_t *rwlock,
 *                         const pthread_rwlockattr_t *attr);
 * int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
 * pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
 *
 * int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
 * int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
 *
 * int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
 * int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
 *
 * int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
 *
 * Это неSpin-блокировка. Это как mutex!
 *
 * ДОБАВИТЬ УСЛОВНЫЕ ПЕРЕМЕННЫЕ БАРЬЕРЫ И ОДНОКРАТНЫЙ ЗАПУСК
 *
 */


 /*
  * Представленная программа использует два потока, печатающих в консоль сообщения,
  * один, печатающий 'a', второй - 'b'. Вывод сообщений смешивается в результате
  * переключения выполнения между потоками или одновременном выполнении на
  * мультипроцессорных системах.
  *
  * Программа создаёт один новый поток для печати 'b', а основной поток
  * печатает 'a'. Основной поток (после печати 'aaaaa….') ждёт завершения дочернего
  * потока.
  *
  * https://ru.wikipedia.org/wiki/POSIX_Threads
  *
  */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void wait_thread (void);
void *thread_func (void *);

int main (/*int argc, char *argv[], char *envp[]*/) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_func, NULL)) { return EXIT_FAILURE; }
    for (unsigned int i = 0; i < 20; i++) {
        puts("a");
        wait_thread();
    }

    if (pthread_join(thread, NULL)) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

void wait_thread (void) {
    time_t start_time = time(NULL);
    while(time(NULL) == start_time) {}
}

void *thread_func (void *vptr_args) {
    for (unsigned int i = 0; i < 20; i++) {
        fputs("b\n", stderr);
        wait_thread();
    }

    return NULL;
}
