#include <stdio.h>
#include <string.h>

int main() {
  char s1[10];
  const char* s2 = "AAAA";
  strcpy(s1, s2);
  return 0;
}
