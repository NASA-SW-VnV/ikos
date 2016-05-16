// SAFE
#include <stdlib.h>

extern int nd();
extern void __ikos_assert(int);

int fst(int* a, int* b) {
  return *a;
}

int snd(int* a, int* b) {
  return *b;
}

int main() {
  int x = 5;
  int y = 7;
  int* p = (int*)malloc(sizeof(int));
  int* q = (int*)malloc(sizeof(int));

  if (nd())
    *p = x;
  else
    *p = y;

  if (nd())
    *q = fst(&x, &y);
  else
    *q = snd(&x, &y);

  __ikos_assert(*p >= 5 && *p <= 7);
  __ikos_assert(*q >= 5 && *q <= 7);

  return 42;
}
