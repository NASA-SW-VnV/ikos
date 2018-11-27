#include <stdint.h>

extern void __ikos_assert(int);

int main() {
  uint64_t x = 0x200000001;
  uint32_t y = (uint32_t)(x);
  __ikos_assert(y == 1);
  return 0;
}
