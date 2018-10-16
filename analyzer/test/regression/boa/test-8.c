// SAFE
#include <stdio.h>
#include <stdlib.h>

// We just use a pointer as an array
#define MAX_ARRAY 10
// To test loops that decrements a counter
int main(int argc, char** argv) {
  int* p = (int*)malloc(sizeof(int) * MAX_ARRAY);
  int i;
  if (!p)
    return 1;
  for (i = MAX_ARRAY - 1; i >= 0; i--) {
    p[i] = i;
  }
  // for underflow check
  // printf("%d\n", p[i+1]);
  // for overflow check
  printf("%d\n", p[MAX_ARRAY - 1]);
  return 0;
}
