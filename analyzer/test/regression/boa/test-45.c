// SAFE
#include <stdio.h>

int* init(int* tab, int n) {
  for (int i = 0; i < n; i++) {
    tab[i] = 0;
  }

  return tab;
}

int main() {
  int tab[10];
  int* p = init(tab, 10);
  printf("%d\n", p[0]);
  printf("%d\n", p[9]);
  return 0;
}
