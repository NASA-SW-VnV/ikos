#include <limits.h>
#include <stdio.h>

extern unsigned __ikos_nondet_uint(void);

int main() {
  unsigned x = __ikos_nondet_uint();
  unsigned y = __ikos_nondet_uint();
  unsigned lim = INT_MAX / 2;
  if (x > lim) {
    x = lim - 1;
  }
  if (y > lim) {
    y = lim - 1;
  }
  unsigned z;
  z = x + y;
  printf("%d", z);
}
