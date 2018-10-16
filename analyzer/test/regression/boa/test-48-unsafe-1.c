#include <stdio.h>
#include <string.h>

int main() {
  const char* s = "AAAAA";
  int i = strlen(s);
  printf("%d\n", (int)s[i + 1]);
  return 0;
}
