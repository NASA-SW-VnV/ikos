#include <stdio.h>
#include <string.h>

extern int __ikos_unknown();

const char* f() {
  if (__ikos_unknown()) {
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
