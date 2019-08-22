// This test performs an access in an unaligned struct, thus resulting in
// undefined behaviour
//
// The access are not volatile to get UBSan check the alignment

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  // reg2 is uint32_t, thus it is not aligned because of the reg1, there
  // should be additionnal padding to use a struct like this
  typedef struct __attribute__((__packed__)) {
    uint8_t reg1;
    uint32_t reg2;
  } twoRegs_t;

  // Malloc to force the structure to be in memory and not optimized inside
  // the registers
  twoRegs_t* twoRegs = malloc(sizeof(twoRegs_t));

  // Perform writing
  twoRegs->reg2 = 30000;

  // Perform a reading
  printf("%p\n", &twoRegs->reg2);
  printf("%i\n", twoRegs->reg2);

  return 0;
}
