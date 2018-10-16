#include <string.h>

typedef struct {
  char buf[10];
  char buf1[10];
  char buf2[10];
} my_struct;

my_struct f(my_struct* s) {
  return *s;
}

my_struct g(my_struct s) {
  return s;
}

int main() {
  return 0;
}
