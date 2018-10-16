#include <stdlib.h>

void fun_exit() {
  int i = 0;
  while (1) {
    if (i == 42)
      exit(1);
    i++;
  }
}

int main() {
  int tab[10];
  int i;

  fun_exit();

  for (i = 0; i <= 10; i++) {
    tab[i] = i * i;
  }
  return tab[0];
}
