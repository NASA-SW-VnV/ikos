#include <string.h>

extern void __ikos_assert(int);

int main() {
  volatile int src[11] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
  int dest[11] = {0};

  memcpy(dest, src, sizeof(int) * 11);

  __ikos_assert(dest[0] == 3);
}
