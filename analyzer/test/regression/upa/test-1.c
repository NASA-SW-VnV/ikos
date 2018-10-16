// SAFE
// This test performs a malloc and tries to access every byte independently
// The access should be aligned on the two accesses
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  // Request 3 bytes, the first is aligned, the second is aligned to 1 byte,
  // not to 2 bytes
  uint8_t* dynAlloc = malloc(3);
  uint8_t* secondByte = dynAlloc + 1;

  printf("Access to %p, value: %x\n", secondByte, *secondByte);

  return 0;
}
