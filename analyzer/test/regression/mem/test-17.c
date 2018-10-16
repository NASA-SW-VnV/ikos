// SAFE
#include <stdlib.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int* array[2];
  int* p1 = (int*)malloc(sizeof(int));
  int* p2 = (int*)malloc(sizeof(int));
  int* p3 = (int*)malloc(sizeof(int));

  if (!p1 || !p2 || !p3)
    return 0;

  *p1 = 9;
  *p2 = 10;
  *p3 = 11;

  array[0] = p1;
  if (__ikos_nondet_int()) {
    array[1] = p2;
  } else {
    array[1] = p3;
  }

  int res1 = *(array[0]);
  int res2 = *(array[1]);

  __ikos_assert(res1 == 9);
  __ikos_assert(res1 >= 9 && res1 <= 11);
  __ikos_assert(res2 >= 10 && res2 <= 11);
  __ikos_assert(res2 >= 9 && res2 <= 11);
  return 42;
}
