// UNSAFE
// global variable is 2-aligned, add 1 so it's unaligned
#include <stdint.h>

extern int __ikos_unknown();

uint16_t* p; // NULL

int main() {
  int a = 1;
  if(__ikos_unknown())
    a = 3;

  uint16_t* b = (uint16_t*)(uint8_t*)p + a ;
  return *b;
}
