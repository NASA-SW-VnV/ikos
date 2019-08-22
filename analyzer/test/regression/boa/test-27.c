#include <stdint.h>
#include <stdio.h>

extern int __ikos_nondet_int(void);
extern unsigned __ikos_nondet_uint(void);
extern void __ikos_assert(int);

char buffer[40] = {0};

void f1() {
  uint8_t i = 40;
  while (i) {
    printf("%d\n", buffer[i - 1]);
    i--;
  }
}

void f1p() {
  uint8_t i = 40;
  while (i != 0) {
    printf("%d\n", buffer[i - 1]);
    i--;
  }
}

void f2() {
  int8_t i = 40;
  while (i) {
    printf("%d\n", buffer[i - 1]);
    i = i - 1;
  }
}

void f2p() {
  int8_t i = 40;
  while (i != 0) {
    printf("%d\n", buffer[i - 1]);
    i = i - 1;
  }
}

void f3() {
  unsigned i = 0;
  while (i != 40) {
    printf("%d\n", buffer[i]);
    i++;
  }
}

int main() {
  f1();
  f1p();
  f2();
  f2p();
  f3();
}
