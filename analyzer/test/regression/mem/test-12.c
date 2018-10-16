// SAFE
#include <stdio.h>
#include <stdlib.h>

extern void __ikos_assert(int);

struct foo {
  int a[5];
  int k;
  int b[10];
};

struct foo* x;

int main(int argc, char** argv) {
  x = (struct foo*)malloc(sizeof(struct foo));
  if (!x)
    return 0;

  x->a[2] = 5;
  x->a[3] = 15;
  x->a[4] = 25;

  int* y = &(x->a[0]);
  y = y + 4;

  x->b[0] = 333;
  x->b[5] = 555;
  int* z = &(x->b[0]);

  __ikos_assert(*y == 25 && *z == 333);
  z = z + 5;
  __ikos_assert(*z == 555 && *z == x->b[5]);

  z = y;
  z++;
  *z = 888;

  __ikos_assert(x->a[5] == 888);
  __ikos_assert(*z == x->a[5]);

  return 42;
}
