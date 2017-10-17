#include <stdio.h>

int MAX_ARRAY = 10;

// To test loops that decrements a counter
int main(int argc, char** argv) {
  int a[MAX_ARRAY];
  int i;
  for (i = MAX_ARRAY - 1; i >= 0; i--) {
    a[i] = i;
  }
  // for underflow check
  printf("%d\n", a[i + 1]);
  // for overflow check
  printf("%d\n", a[MAX_ARRAY]);
  return 0;
}
