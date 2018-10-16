#include <stdlib.h>

extern int __ikos_nondet_int(void);

int* f() {
  int array[10];
  for (int i = 0; i < 10; i++) {
    array[i] = __ikos_nondet_int();
  }

  for (int i = 0; i < 10; i++) {
    if (array[i] == 42) {
      return &array[i];
    }
  }

  return NULL;
}

int main() {
  int* ret = f();
  if (ret != NULL) {
    return *ret;
  } else {
    return 0;
  }
}
