#include <stdint.h>

extern int __ikos_nondet_int(void);
extern unsigned int __ikos_nondet_uint(void);
extern uint64_t f(void);

int main() {
  int* c;

  c[4] = 5;
}
