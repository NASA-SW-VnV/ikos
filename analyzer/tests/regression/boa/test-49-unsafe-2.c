#include <stdio.h>
#include <string.h>

int main() {
  char s1[4];
  const char* s2 = "AAAA";
  strcpy(s1, s2);
  return 0;
}
