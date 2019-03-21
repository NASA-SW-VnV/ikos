#include <stdio.h>
#include <string.h>

int main() {
  char buffer[32];
  strncpy(buffer, "hello", sizeof(buffer));
  puts(buffer);
  return 0;
}
