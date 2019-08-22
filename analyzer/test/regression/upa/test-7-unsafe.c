// UNSAFE
// Complex function that gives an offset
#include <stdint.h>
#include <stdio.h>

uint8_t foo(uint8_t k, uint8_t N) {
  return k + N;
}

uint8_t bar(uint8_t p) {
  return p + foo(p, 10);
}

int main(int argc, char** argv) {
  uint8_t a[56];
  uint8_t x = foo(5, 10); // x=15
  uint8_t y = x + bar(x); // y=55
  a[x] = y;

  uint16_t* test;
  test = a[x] + y + 1;
  return *test;
}
