// SAFE

#include <stdio.h>
#define MAX_ARRAY 10

struct bar {
  int x;
  float y;
};

struct foo {
  int x;
  struct bar y;
  int a[MAX_ARRAY][MAX_ARRAY][MAX_ARRAY - 1];
};

// To test loops
int main(int argc, char** argv) {
  int i, j, k;
  struct foo x;
  for (i = 0; i < MAX_ARRAY; i++) {
    for (j = 0; j < MAX_ARRAY; j++) {
      for (k = 0; k < MAX_ARRAY - 1; k++) {
        x.a[i][j][k] = argc; // some unknown value here
        x.y.x = x.a[i][j][k];
      }
    }
  }

  for (i = 0; i < MAX_ARRAY; i++) {
    printf("%d\n", x.a[i][i][i - 1]);
  }

  return 0;
}
