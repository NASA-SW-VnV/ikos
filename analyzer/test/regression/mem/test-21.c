// SAFE
#include <stdlib.h>

int main() {
  int n;
  int **p, *r;
  int* a[10];

  for (n = 0; n < 10; n++) {
    p = &a[n];
    r = (int*)malloc(sizeof(int));
    if (r == NULL) {
      return 0;
    }
    *r = n;
    *p = r;
  }

  r = a[9];
  return *r;
}
