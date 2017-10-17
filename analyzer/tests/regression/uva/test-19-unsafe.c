#include <string.h>

void f(int* p) {
  memset(p, 0, 10);
}

int main() {
  int* p;
  f(p);
  return 0;
}
