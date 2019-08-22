#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int x = 0;

  int* p;
  if (__ikos_nondet_int()) {
    p = &x;
  } else {
    p = NULL;
  }

  int* q = &x;

  if (p != q) {
    __ikos_assert(1);
  } else {
    __ikos_assert(1);
  }

  return 0;
}
