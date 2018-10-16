#include <stdio.h>
#include <stdlib.h>

#define HWADDR_1 0x40

int main() {
  (*(int*)HWADDR_1) = 0x41;
}
