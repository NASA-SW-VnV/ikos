#include <stdint.h>

int f() {
  return 0;
}

uintptr_t x = (intptr_t)&f;
