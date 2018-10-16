// SAFE
#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int *p, *q, *r;

  p = (int*)malloc(sizeof(int));
  q = (int*)malloc(sizeof(int));
  r = (int*)malloc(sizeof(int));

  if (!p || !q || !r)
    return 0;

  *q = 3;
  *r = 5;

  if (__ikos_nondet_int())
    p = q;
  else
    p = r;

  __ikos_assert(*p >= 3 && *p <= 5);
  return *p;
}
