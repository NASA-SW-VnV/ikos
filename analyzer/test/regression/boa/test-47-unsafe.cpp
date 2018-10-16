#include <cstdio>

extern "C" {
extern int __ikos_nondet_int(void) noexcept;
}

int x;

void f() {
  x = 10;
  if (__ikos_nondet_int()) {
    throw 0x42;
  }
  x = 0;
}

int main() {
  int tab[10] = {0};

  try {
    f();
  } catch (int) {
  }

  printf("%d\n", tab[x]);
}
