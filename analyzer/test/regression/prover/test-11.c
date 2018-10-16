#include <stdarg.h>

extern void __ikos_assert(int);

int foo(int n, ...) {
  int* p;
  int r = 0;
  va_list ap;
  va_start(ap, n);
  for (int i = 0; i < n; i++) {
    p = va_arg(ap, int*);
    r += *p;
  }
  va_end(ap);
  return r;
}

int main() {
  int x = 1, y = 2, z = 3;
  int r = foo(2, &x, &y, &z);
  __ikos_assert(r >= 0);
  __ikos_assert(r == 3);
  return 0;
}
