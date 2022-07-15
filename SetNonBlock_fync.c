/*<==============================================================================>*/
/*                             SetNonBlock_function                               */
/*                      Функция делает сокет неблокирующим                        */
/*<==============================================================================>*/
#include<fcntl.h>

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
  return 0;
}
