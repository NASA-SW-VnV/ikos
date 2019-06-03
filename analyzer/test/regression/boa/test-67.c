#include <stdint.h>

extern int __ikos_nondet_int(void);

static uint8_t x = 0;

int main() {
  int A[10];
  while (__ikos_nondet_int()) {
    x++;
    if (x >= 10) {
      x = 0;
    }
    A[x] = __ikos_nondet_int();
  }
  A[x] = __ikos_nondet_int();
  return 0;
}
