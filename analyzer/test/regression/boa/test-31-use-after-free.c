#include <stdio.h>
#include <stdlib.h>

extern int __ikos_nondet_int(void);

int main() {
  int* p = calloc(10, sizeof(int));
  if (p != NULL) {
    if (__ikos_nondet_int()) {
      free(p);
    } else {
      p[5] = __ikos_nondet_int();
    }
    printf("p=%d\n", p[5]);
  }
}
