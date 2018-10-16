#include <stdio.h>

extern unsigned __ikos_nondet_uint(void);

int main() {
  unsigned a = __ikos_nondet_uint();
  if (a > 1000) {
    return 0;
  }
  unsigned b = a * 8 + 42;

  unsigned c = a << b;
  return c;
}
