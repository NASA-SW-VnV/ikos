#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern int __ikos_nondet_int(void);
extern unsigned int __ikos_nondet_uint(void);

bool f(int* array_a, int* array_b, int pos_a, int pos_b) {
  return &array_a[pos_a] < &array_b[pos_b];
}

int main() {
  int* array_a = calloc(10, sizeof(int));
  int* array_b = calloc(10, sizeof(int));
  int* final_array;
  int pos_a = __ikos_nondet_uint() % 10;
  int pos_b = __ikos_nondet_uint() % 10;
  bool b;
  if (__ikos_nondet_int()) {
    final_array = array_a;
  } else {
    final_array = array_b;
  }
  b = f(final_array, array_b, pos_a, pos_b);

  free(array_a);
  free(array_b);

  return b;
}
