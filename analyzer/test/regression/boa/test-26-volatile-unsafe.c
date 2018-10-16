#include <stdio.h>

extern unsigned __ikos_nondet_uint(void);

int main() {
  int values[5] = {0};
  // volatile should trigger a warning
  volatile unsigned i = __ikos_nondet_uint() % 5;
  printf("%d\n", values[i]);
}
