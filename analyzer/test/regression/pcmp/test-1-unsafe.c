#include <stdlib.h>

int main(void) {
  char* p = malloc(42);
  char* q = malloc(42);

  int tmp = p < q; // Error

  free(p);
  free(q);

  return tmp;
}
