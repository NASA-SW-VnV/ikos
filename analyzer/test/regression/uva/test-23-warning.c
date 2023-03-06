#include <stdlib.h>
int main(int argc, char** argv) {
  int* p = NULL;
  int x = 0;
  p = malloc(sizeof(int)); // uninitialized
  if (!p) {
    return 0;
  }
  if (argc) {
    p = &x;
  }
  int y = *p; // initialized if p = &x but uninitialized if p = malloc(..)
  return y;
}
