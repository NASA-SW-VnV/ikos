#include <stddef.h>

struct S {
  char* p;
  char c;
};

int main() {
  struct S s = {NULL, 'a'};
  return 0;
}
