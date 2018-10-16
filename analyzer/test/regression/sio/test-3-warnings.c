#include <limits.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int x, y, z;
  x = __ikos_nondet_int();
  y = __ikos_nondet_int();
  z = x + y;
  printf("%d", z);
}
