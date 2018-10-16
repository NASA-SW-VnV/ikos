// UNSAFE
#include <stdio.h>

int* init() {
  int tab[10];
  for (int i = 0; i < 10; i++) {
    tab[i] = 0;
  }

  return tab; // unsafe: return a local variable
}

int main() {
  int* p = init();
  printf("%d\n", p[0]);
  printf("%d\n", p[9]);
  return 0;
}
