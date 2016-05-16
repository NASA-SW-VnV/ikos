// SAFE
#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_unknown();

int* foo(int* a, int* b) {
  int* res;
  if (__ikos_unknown())
    res = a;
  else
    res = b;
  return res;
}

int main() {
  int *p, *q, *r;
  int *a, *b, *c;

  p = (int*)malloc(sizeof(int));
  q = (int*)malloc(sizeof(int));
  r = (int*)malloc(sizeof(int));

  a = (int*)malloc(sizeof(int));
  b = (int*)malloc(sizeof(int));
  c = (int*)malloc(sizeof(int));

  *q = 3;
  *r = 5;

  *b = 10;
  *c = 15;

  p = foo(q, r);

  a = foo(b, c);

  __ikos_assert(*p >= 3 && *p <= 5);
  __ikos_assert(*a >= 10 && *a <= 15);
  return *p;
}
