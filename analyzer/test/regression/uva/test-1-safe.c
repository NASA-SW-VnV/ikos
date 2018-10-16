#include <stdio.h>

int main(int argc, char** argv) {
  int k, i;
  k = 0;
  for (i = 0; i < 10; i++) {
    k = k + 1;
  }
  printf("%d", k);
  return 0;
}
