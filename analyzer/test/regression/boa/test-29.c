#include <stdio.h>
#include <stdlib.h>

extern int __ikos_nondet_int(void);

int main() {
  int* hwaddr;
  if (__ikos_nondet_int()) {
    hwaddr = 0x42;
  } else if (__ikos_nondet_int() == 42) {
    hwaddr = 0x47;
  } else {
    hwaddr = 0x99;
  }

  *hwaddr = 142857;
}
