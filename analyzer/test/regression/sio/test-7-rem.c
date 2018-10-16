#include <limits.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int x = INT_MIN;
  int y = -1;
  int z = x % y;

  printf("%d", z);

  return z;
}
