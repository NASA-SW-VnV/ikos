#include <limits.h>
#include <stdio.h>

extern unsigned __ikos_nondet_uint(void);

int main() {
  unsigned x = __ikos_nondet_uint();
  unsigned y = __ikos_nondet_uint();
  unsigned z;
  if (x >= UINT_MAX - 1 && y >= 2) {
    z = x + y;
  } else {
    z = 42;
  }
  printf("%d", z);
}
