#include <stdio.h>
#include <string.h>

char buf[10];

extern int __ikos_unknown();

char* f() {
  if (__ikos_unknown()) {
    return buf;
  } else {
    return NULL;
  }
}

int main() {
  char* s1 = f();
  const char* s2 = "AAAA";
  strcpy(s1, s2);
  return 0;
}
