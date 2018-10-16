#include <stdio.h>
#include <string.h>

extern int __ikos_nondet_int(void);

const char* f() {
  if (__ikos_nondet_int()) {
    return "BBBB";
  } else {
    return NULL;
  }
}

int main() {
  char s1[9] = "AAAA";
  const char* s2 = f();
  strcat(s1, s2);
  return 0;
}
