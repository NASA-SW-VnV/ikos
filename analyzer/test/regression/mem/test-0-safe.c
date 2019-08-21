// SAFE
#include <stddef.h>
#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int* p = (int*)malloc(sizeof(int));
  if (p == NULL)
    exit(0);

  *p = 0;
  while (__ikos_nondet_int()) {
    if (__ikos_nondet_int())
      *p = 1;
    if (__ikos_nondet_int())
      *p = 5;
  }
  __ikos_assert(*p >= 0 && *p <= 5);
  return 42;
}
