// Test that generates extractelem, insertelem and range for a struct
#include <stdio.h>

typedef struct {
  int x1;
  int x2;
  int x3;
} vector_t;

vector_t f() {
  vector_t v;
  v.x1 = 1;
  v.x2 = 2;
  v.x3 = 3;
  return v;
}

void print_vector(vector_t v) {
  printf("%d %d %d\n", v.x1, v.x2, v.x3);
}

int main() {
  vector_t v = f();
  print_vector(v);
  return 0;
}
