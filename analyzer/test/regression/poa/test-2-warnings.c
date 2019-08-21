#include <stdint.h>

extern int __ikos_nondet_int(void);
extern unsigned int __ikos_nondet_uint(void);
extern uint64_t f(void);

int main() {
  int c[40];
  uint64_t a = UINT64_MAX / 4 + __ikos_nondet_int();
  c[a] = 5;
}
