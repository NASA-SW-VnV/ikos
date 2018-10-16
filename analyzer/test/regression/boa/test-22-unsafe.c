// DEFINITE UNSAFE
#include <stdio.h>

struct foo {
  int a;
  int b;
};

struct foo x;

int main(int argc, char** argv) {
  int i;
  int a[10];
  for (i = 0; i < 10; i++) {
    x.a = i + 1;
    a[x.a] = i;
  }
  printf("%d\n", a[i - 1]);
  return 42;
}
