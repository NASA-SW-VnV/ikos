// DEFINITE UNSAFE (overflow)

#include <stdio.h>
#define MAX_ARRAY 10

// To test loops
int main(int argc, char** argv) {
  int i, j;
  int a[MAX_ARRAY][MAX_ARRAY];
  for (i = 0; i < MAX_ARRAY; i++) {
    for (j = 0; j < MAX_ARRAY; j++)
      a[i + 1][j] = argc; // some unknown value here
  }

  for (i = 0; i < MAX_ARRAY; i++) {
    printf("%d\n", a[i][i]);
  }

  return 0;
}
