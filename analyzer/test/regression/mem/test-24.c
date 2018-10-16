// SAFE
#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

struct foo {
  int* p;
  int* q;
};

int main() {
  struct foo* x = (struct foo*)malloc(sizeof(struct foo));
  if (!x)
    return 0;

  x->p = (int*)malloc(sizeof(int));
  x->q = (int*)malloc(sizeof(int));

  if (!(x->p) || !(x->q))
    return 0;

  *(x->p) = 9;
  *(x->q) = 20;

  int *p1, *p2;

  if (__ikos_nondet_int())
    p1 = x->p;
  else
    p1 = x->q;

  p2 = x->q;

  __ikos_assert(*p1 >= 9 && *p1 <= 20);
  __ikos_assert(*p2 == 20);
  return *p1 + *p2;
}
