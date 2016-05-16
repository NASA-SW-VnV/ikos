// SAFE
#include <stdlib.h>
#include <stdint.h>

extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int* p = (int*)malloc(sizeof(int));
  if (p == 0)
    exit(0);

  *p = 0;
  while (__ikos_unknown()) {
    if (__ikos_unknown())
      *p = 1;
    if (__ikos_unknown())
      *p = 5;
  }
  __ikos_assert(*p >= 0 && *p <= 5);
  return 42;
}
