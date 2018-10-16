#include <stdlib.h>

extern int __ikos_nondet_int(void);

int main() {
  int* p;
  p = (int*)calloc(1, sizeof(int));
  if (p != NULL) {
    *p = __ikos_nondet_int();
    free(p);
    free(p);
  }
}
