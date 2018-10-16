// UNSAFE
// This test performs a malloc and tries to access every byte independently
// The access should be aligned on the first access, but not on the following
//
// The Misaligned access is caused by setting the an uint16_t pointer to the
// second byte of a malloc. To remove the incompatible-pointer-type warning,
// the malloc pointer is casted to a void*
//
// The undefined behaviour appears on the pointer cast operated: dynAlloc is of
// type uint8_t (1 byte), while the secondByte pointer is of type uint16_t, thus
// making an implicit cast from a 8b to a 16b pointer. See (6.3.2.3.7) of
// ISO/IEC 9899:TC2 for more details.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  // Request 3 bytes, the first is aligned, the second is aligned to 1 byte,
  // not to 2 bytes
  uint8_t* dynAlloc = malloc(3);
  uint16_t* secondByte = (void*)(dynAlloc + 1);

  printf("Access to %p, value: %x\n", secondByte, *secondByte);

  return 0;
}
