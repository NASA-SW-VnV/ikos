#include <stdio.h>
#include <stdlib.h>

extern void __ikos_assert(int);

int* f() {
  int* p = (int*)malloc(sizeof(int));

  if (p == NULL) {
    exit(0);
  }

  return p;
}

int main() {
  int* p = f();
  *p = 0;

  int* q = f();
  *q = 42;

  __ikos_assert(p != q);
  __ikos_assert(*p == 0);
  __ikos_assert(*q == 42);
}
