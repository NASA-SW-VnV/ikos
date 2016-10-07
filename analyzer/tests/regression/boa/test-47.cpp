#include <cstdio>

extern int nd();

int x;

void f() {
  x = 9;
  if (nd()) {
    throw nullptr;
  }
  x = 0;
}

int main() {
  int tab[10];

  try {
    f();
  } catch (void*) {
  }

  printf("%d\n", tab[x]);
}
