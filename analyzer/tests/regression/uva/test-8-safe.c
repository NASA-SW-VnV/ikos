#include <stdio.h>

extern int __ikos_unknown();

int main(int argc, char** argv) {
  argc = __ikos_unknown();

  if (argc < 0)
    return 42;

  int x, y;
  if (argc == 0)
    x = 0;
  else if (argc == 1)
    x = 1;
  else if (argc > 1)
    x = 2;

  return x;
}
