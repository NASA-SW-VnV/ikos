#include <stdio.h>
#include <string.h>

int main() {
  const char* s = "AAAAA";
  int i = strlen(s);
  printf("%d\n", (int)s[i]);
  return 0;
}
