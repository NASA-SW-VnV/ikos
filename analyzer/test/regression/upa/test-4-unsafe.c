// SAFE
// Adds odd number to a uint16_t address, resulting in unsafe access
#include <stdint.h>

extern int __ikos_nondet_int(void);

uint16_t* foo(uint8_t* x, uint8_t* y) {
  return (uint16_t*)(x + *y);
}

int main() {
  uint16_t a = 0;

  uint8_t b = 3;
  if (__ikos_nondet_int())
    b = 5 * b;
  else
    b = 7;

  uint16_t* z = foo((uint8_t*)&a, &b);
  return *z;
}
