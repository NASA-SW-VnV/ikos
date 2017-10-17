// SAFE
#include <memory.h>

struct Foo {
  int a;    // 4
  int b;    // 4
  int c[5]; // 20
  char* s;  // 8
  // + padding of 4 in my 64bit machine
}; // sizeof (struct Foo) = 40

int foo(struct Foo f) {
  return f.b;
}

int main() {
  struct Foo f;
  memset(&f, 0, sizeof(f));
  return foo(f);
}
