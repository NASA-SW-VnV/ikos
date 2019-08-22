#include <stdarg.h>
#include <stdio.h>

extern void __ikos_assert(int);

long foo(long n, ...) {
  long r = 0;
  va_list ap;
  va_start(ap, n);
  for (long i = 0; i < n; i++) {
    r += va_arg(ap, long);
  }
  va_end(ap);
  return r;
}

int main() {
  long r = foo(4, 1, 2, 3, 4);
  __ikos_assert(r == 10);
  return 0;
}
