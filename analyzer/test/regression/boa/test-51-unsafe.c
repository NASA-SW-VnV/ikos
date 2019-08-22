#include <stdarg.h>
#include <stdio.h>

void foo(const char* fmt, ...) {
  va_list ap;
  int d;
  char c, *s;
  double f;

  va_start(ap, fmt);
  while (*fmt) {
    switch (*fmt++) {
      case 's': /* string */
        s = va_arg(ap, char*);
        printf("string %s\n", s);
        break;
      case 'd': /* int */
        d = va_arg(ap, int);
        printf("int %d\n", d);
        break;
      case 'c': /* char */
        /* need a cast here since va_arg only takes fully promoted types */
        c = (char)va_arg(ap, int);
        printf("char %c\n", c);
        break;
      case 'f': /* float */
        f = va_arg(ap, double);
        printf("double %f\n", f);
        break;
      default:
        break;
    }
  }
  va_end(ap);
}

int main(int argc, char** argv) {
  if (argc >= 2) {
    foo(argv[1]); // similar to a format string vulnerability
  }
  return 0;
}
