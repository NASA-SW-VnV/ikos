#include <stdio.h>
#include <string.h>

extern int nd();

const char* f() {
  if (nd()) {
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
