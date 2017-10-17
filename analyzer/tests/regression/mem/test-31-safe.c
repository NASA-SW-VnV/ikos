#include <stdint.h>
#include <stdlib.h>

extern void __ikos_assert(int);

int main() {
  int** a = malloc(sizeof(int*));
  if (a == NULL)
    return 1;

  *a = NULL;
  __ikos_assert(*((int64_t*)a) == 0);

  int64_t* b = malloc(sizeof(int64_t));
  if (b == NULL)
    return 1;

  *b = 0;
  __ikos_assert(*(int**)(b) == NULL);

  return 0;
}
