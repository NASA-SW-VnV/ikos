#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int a = __ikos_nondet_int();
  int b = __ikos_nondet_int();
  if (b > 0) {
    b = -b;
  } else if (b == 0) {
    b = -1;
  }
  int c = a << b;

  return c;
}
