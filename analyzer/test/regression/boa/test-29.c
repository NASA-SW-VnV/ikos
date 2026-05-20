#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int __ikos_nondet_int(void);

int main() {
  int* hwaddr;
  if (__ikos_nondet_int()) {
    hwaddr = (int*)(uintptr_t)0x42;
  } else if (__ikos_nondet_int() == 42) {
    hwaddr = (int*)(uintptr_t)0x47;
  } else {
    hwaddr = (int*)(uintptr_t)0x99;
  }

  *hwaddr = 142857;
}
