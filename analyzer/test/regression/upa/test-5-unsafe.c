// UNSAFE
// global variable is 2-aligned, add 1 so it's unaligned
#include <stdint.h>

extern int __ikos_nondet_int(void);

uint16_t x;

int main() {
  int a = 1;
  if (__ikos_nondet_int())
    a = 3;

  uint16_t* b = (uint16_t*)((uint8_t*)&x + a);
  return *b;
}
