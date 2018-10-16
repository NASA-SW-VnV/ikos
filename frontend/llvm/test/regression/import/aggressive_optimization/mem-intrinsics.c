#include <string.h>

int cst() {
  int G = 0 ? 0 : (((1 + 1) + 3) * 2);
  return G;
}

int main() {
  int *p, *q, *r;
  r = (int*)memcpy(p, q, 10);
  r = (int*)memmove(p, q, 50);
  r = (int*)memset(p, 1, 50);
}
