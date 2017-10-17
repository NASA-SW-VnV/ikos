#include <stdio.h>
#include <string.h>

int main() {
  char s1[10] = "A";
  const char* s2 = "BBBB";
  strcat(s1, s2);
  return 0;
}
