// SAFE

#include <stdio.h>

// To test loops
int main(int argc, char** argv) {
  int i;
  int a[10];
  for (i = 0; i < 10; i++) {
    a[i] = i;
  }
  printf("%d\n", a[i - 1]);
  return 0;
}
