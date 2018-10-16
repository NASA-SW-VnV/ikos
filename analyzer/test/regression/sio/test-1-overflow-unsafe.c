#include <limits.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int x = __ikos_nondet_int();
  int y = __ikos_nondet_int();
  int z;
  if (x >= INT_MAX - 1 && y >= 2) {
    z = x + y;
  } else {
    z = 42;
  }
  printf("%d", z);
}
