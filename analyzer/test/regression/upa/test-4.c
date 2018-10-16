// SAFE
// Adds even number to a uint16_t address, still resulting in safe access
#include <stdint.h>

extern int __ikos_nondet_int(void);

uint16_t* foo(uint16_t* x, uint16_t* y) {
  return x + *y;
}

int main() {
  uint16_t a = 0;

  uint16_t b = 2;
  if (__ikos_nondet_int())
    b = 4 * b;
  else
    b = 6;

  uint16_t* z = foo(&a, &b);
  return *z;
}
