#include <string.h>

void f(int* p) {
  char tab[10];
  memcpy(tab, p, 105);
}

int main() {
  int* p;
  f(p);
  return 0;
}
