#include <stdio.h>
#include <stdlib.h>

typedef void* (*fun_ptr_t)(size_t);

char buffer[6];

void* my_malloc(size_t s) {
  return &buffer;
}

extern int nd();

int main() {
  fun_ptr_t f;

  if (nd()) {
    f = &malloc;
  } else {
    f = &my_malloc;
  }

  int* p = (int*)f(2 * sizeof(int));
  *p = 1;
  p++;
  *p = 2;

  printf("%d\n", *p);
  return 0;
}
