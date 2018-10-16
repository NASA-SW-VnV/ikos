#include <string.h>

struct X;

typedef struct {
  int* a;
  struct X* b;
} my_struct;

my_struct s;

int main() {
  return 0;
}
