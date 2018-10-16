#include <stdio.h>

extern int __ikos_nondet_int(void);

int main(int argc, char** argv) {
  argc = __ikos_nondet_int();

  if (argc < 0)
    return 42;

  int x;
  if (argc == 0)
    x = 0;
  else if (argc == 1)
    x = 1;
  else if (argc > 1)
    x = 2;

  return x;
}
