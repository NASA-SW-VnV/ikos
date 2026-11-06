// SAFE
#include <string.h>

extern int __ikos_nondet_int(void);

int main() {
  char buf[16];
  memset(buf, __ikos_nondet_int(), sizeof(buf));
  return buf[0];
}
