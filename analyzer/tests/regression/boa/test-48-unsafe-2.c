#include <stdio.h>
#include <string.h>

int main() {
  const char* s = "AAAAA";
  int i = strlen(&s[6]);
  printf("%d\n", (int)s[i]);
  return 0;
}
