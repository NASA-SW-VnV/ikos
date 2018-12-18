#include <stdio.h>

typedef struct {
  float x;
  float y;
} pos_t;

typedef struct {
  pos_t begin, end;
} line_t;

line_t f(float y) {
  return {{0.0, y}, {2.0, 0}};
}

int main() {
  printf("%f\n", f(2.0).begin.y);
  return 0;
}
