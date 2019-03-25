#include <errno.h>
#include <stdio.h>

extern void __ikos_assert(int);

int main() {
  errno = 0;
  printf("hello world\n");
  __ikos_assert(errno == 0); // errno could be != 0, assertion is wrong
  return 0;
}
