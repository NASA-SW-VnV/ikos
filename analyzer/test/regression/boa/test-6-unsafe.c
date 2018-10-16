// DEFINITE SAFE
#include <stdio.h>
#define MAX_ARRAY 10

struct foo {
  int x;
  int a[MAX_ARRAY][MAX_ARRAY];
};

// To test loops
int main(int argc, char** argv) {
  int i, j;
  struct foo x;
  for (i = 0; i < MAX_ARRAY; i++) {
    for (j = 0; j < MAX_ARRAY; j++)
      x.a[i][j] = argc; // some unknown value here
  }

  for (i = 0; i < MAX_ARRAY; i++) {
    printf("%d\n", x.a[i][i + 1]);
  }

  return 0;
}
