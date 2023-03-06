#include <stdlib.h>
int main() {
  char* p = malloc(sizeof(char));
  if (!p) {
    return 5;
  }
  *p = 3;
  return (int)*p;
}
