// DEFINITE UNSAFE
#include <stdint.h>

// all zeros

extern void __ikos_assert(int);

int main() {
  int a[10];
  int64_t i;

  for (i = 0; i < 10; i++)
    a[i] = 0;

  __ikos_assert(a[9] == 5);
  return 42;
}
