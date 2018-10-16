#include <limits.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int x = __ikos_nondet_int();
  int y = __ikos_nondet_int();
  int lim = INT_MAX / 2;
  if (x > lim || x < 0) {
    x = lim - 1;
  }
  if (y > lim || y < 0) {
    y = lim - 1;
  }
  int z;
  z = x + y;
  printf("%d", z);
}
