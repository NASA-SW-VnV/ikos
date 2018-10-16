#include <stdio.h>

extern int __ikos_nondet_int(void);

int main() {
  int a = __ikos_nondet_int();
  int b = __ikos_nondet_int();

  if (b > 0) {
    b = -b;
  }
  int c = a >> b;

  return c;
}
