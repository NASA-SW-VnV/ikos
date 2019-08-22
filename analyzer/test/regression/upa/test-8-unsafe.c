#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void foo(const char* fmt, ...) {
  va_list ap;
  uint8_t* a;
  uint16_t* b;
  uint32_t* c;

  va_start(ap, fmt);
  while (*fmt) {
    switch (*fmt++) {
      case '8': /* uint8_t */
        a = va_arg(ap, uint8_t*);
        printf("int %d\n", *a);
        break;
      case '1': /* uint16_t */
        b = va_arg(ap, uint16_t*);
        printf("int %d\n", *b);
        break;
      case '3': /* uint32_t */
        c = va_arg(ap, uint32_t*);
        printf("int %d\n", *c);
        break;
      default:
        break;
    }
  }
  va_end(ap);
}

int main() {
  uint32_t a[10] = {0};

  foo("813", ((char*)&a[0]) + 1, ((char*)&a[0]) + 1, ((char*)&a[0]) + 1);
  return 0;
}
