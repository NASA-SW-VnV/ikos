// SAFE
#include <stdlib.h>

int main() {
  int n = 0;
  int **q, **p, *r;
  int* a[10];

  while (n < 10) {
    q = &a[0];
    p = q + n;
    r = (int*)malloc(sizeof(int));
    *p = r;
    n = n + 1;
  }
  return (long long)p[0];
}
