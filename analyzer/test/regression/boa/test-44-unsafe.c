#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef void* (*fun_ptr_t)(size_t);

uint64_t buffer[6];

void* my_malloc(size_t s) {
  return &buffer;
}

extern int __ikos_nondet_int(void);

int main() {
  fun_ptr_t f;

  if (__ikos_nondet_int()) {
    f = &malloc;
  } else {
    f = &my_malloc;
  }

  int* p = (int*)f(2 * sizeof(int));
  if (!p)
    return 1;
  *p = 1;
  p++;
  *p = 2;

  printf("%d\n", *p);
  return 0;
}
