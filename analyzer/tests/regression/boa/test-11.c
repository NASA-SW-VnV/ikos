// SAFE if interprocedural
#include <stdio.h>

int a[100][100];
int init = 5;

int foo(int k) {
  while (k < 100) {
    int i = 0;
    int j = k;
    while (i < j) {
      i = i + 1;
      j = j - 1;
      a[i][j] = init;
    }
    k = k + 1;
  }
  return k;
}

int main(int argc, char** argv) {
  int ind = foo(0);
  printf("%d\n", a[ind - 1][ind - 1]);
}
