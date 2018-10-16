#include <stdio.h>

extern unsigned __ikos_nondet_uint(void);

int main() {
  int values[5] = {0};
  // not volatile
  unsigned i = __ikos_nondet_uint() % 5;
  printf("%d\n", values[i]);
}
