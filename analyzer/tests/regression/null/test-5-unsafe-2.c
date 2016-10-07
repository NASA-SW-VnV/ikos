#include <stdio.h>
#include <string.h>

char buf[10] = "AAAA";

extern int nd();

char* f() {
  if (nd()) {
    return buf;
  } else {
    return NULL;
  }
}

int main() {
  char* s1 = f();
  const char* s2 = "BBBB";
  strcat(s1, s2);
  return 0;
}
