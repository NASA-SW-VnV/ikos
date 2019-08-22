// SAFE
#include <stdio.h>
#include <stdlib.h>

int main() {
  int* p = (int*)malloc(sizeof(int));
  if (!p)
    return 1;
  *p = 0x41424344;
  short* q = (short*)((char*)p + 2);
  *q = 0;
  return 0;
}
