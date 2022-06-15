#include <stdlib.h>
int main() {
  int* p = (int*)malloc(sizeof(int));
  if (!p) {
    return 0;
  }
  return *p + 5;
}
