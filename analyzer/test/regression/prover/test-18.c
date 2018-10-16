#include <stdint.h>

extern void __ikos_assert(int);

int main() {
  uint16_t k;

  for (k = 0; k < 60000; k++) {
    asm volatile("nop");
  }

  __ikos_assert(1);
  __ikos_assert(k == 60000);
  return 0;
}
