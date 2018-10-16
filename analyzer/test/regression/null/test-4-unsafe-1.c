#include <stdio.h>
#include <string.h>

extern int __ikos_nondet_int(void);

const char* f() {
  if (__ikos_nondet_int()) {
    return "AAAA";
  } else {
    return NULL;
  }
}

int main() {
  char s1[10];
  const char* s2 = f();
  strcpy(s1, s2);
  return 0;
}
