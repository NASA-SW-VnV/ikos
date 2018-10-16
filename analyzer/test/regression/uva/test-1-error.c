#include <stdio.h>

extern int __ikos_nondet_int(void);

int main(int argc, char** argv) {
  argc = __ikos_nondet_int();

  if (argc > 0) {
    int k, i;
    for (i = 0; i < 10; i++) {
      k = k + 1;
    }
    printf("%d", k);
  }
  return 0;
}
