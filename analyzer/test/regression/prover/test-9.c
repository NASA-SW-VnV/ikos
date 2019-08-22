#include <stdarg.h>
#include <stdio.h>

extern void __ikos_assert(int);

long foo(long a, ...) {
  long r = a;
  va_list ap;
  va_start(ap, a);
  r += va_arg(ap, long);
  r += va_arg(ap, long);
  r += va_arg(ap, long);
  va_end(ap);
  return r;
}

int main() {
  long r = foo(1, 2, 3, 4);
  __ikos_assert(r == 10);
  return 0;
}
