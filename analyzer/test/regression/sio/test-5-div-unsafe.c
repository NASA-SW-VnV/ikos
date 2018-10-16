#include <limits.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int x, y, z;
  x = INT_MIN;
  y = -1;
  z = x / y;
  printf("%d", z);
}
