#include <cstdio>

extern int __ikos_unknown();

int x;

void f() {
  x = 9;
  if (__ikos_unknown()) {
    throw nullptr;
  }
  x = 0;
}

int main() {
  int tab[10] = {0};

  try {
    f();
  } catch (void*) {
  }

  printf("%d\n", tab[x]);
}
