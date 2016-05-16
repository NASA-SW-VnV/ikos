#include <stdio.h>

extern int unknown();

int main(int argc, char** argv) {
  argc = unknown();

  if (argc > 0) {
    int k, i;
    for (i = 0; i < 10; i++) {
      k = k + 1;
    }
    printf("%d", k);
  }
  return 0;
}
