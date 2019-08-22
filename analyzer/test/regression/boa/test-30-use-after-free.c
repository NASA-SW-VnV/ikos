#include <stdlib.h>

int main() {
  int* p = calloc(10, sizeof(int));
  if (p != NULL) {
    p[5] = 17;
    free(p);
    p[5] = 18;
  }
}
