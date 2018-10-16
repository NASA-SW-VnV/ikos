// SAFE
#include <stdio.h>
#include <stdlib.h>

extern void __ikos_assert(int);

struct foo {
  int a;
  int b;
};

struct foo* x;

int main(int argc, char** argv) {
  x = (struct foo*)malloc(sizeof(struct foo));
  if (!x)
    return 0;

  x->a = 5;
  x->b = x->a + 7;
  x->a++;

  // printf("%d %d\n",x->a, x->b);
  __ikos_assert(x->a == 6 && x->b == 12);

  /*
  x->a = 5;
  x->b = 10;
  __ikos_assert (x->a == 5 && x->b == 10);
  */

  return 42;
}
