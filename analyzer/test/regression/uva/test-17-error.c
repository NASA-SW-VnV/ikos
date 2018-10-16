#include <string.h>

void f(int* p) {
  char tab[10];
  memcpy(p, tab, 105);
}

int main() {
  int* p;
  f(p);
  return 0;
}
