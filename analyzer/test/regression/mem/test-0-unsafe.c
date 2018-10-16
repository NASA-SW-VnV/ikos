// DEFINITE UNSAFE

#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int* p = (int*)malloc(sizeof(int));
  if (p == 0)
    exit(0);

  *p = 0;
  while (__ikos_nondet_int()) {
    if (__ikos_nondet_int())
      *p = 1;
    if (__ikos_nondet_int())
      *p = 3;
  }
  __ikos_assert(*p == 4);
  return 42;
}
