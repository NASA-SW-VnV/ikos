#include <stdio.h>

int main() {
  char buffer[32];
  FILE* f = fopen("passwd", "r");
  if (f == NULL) {
    puts("error");
    return -1;
  }
  fgets(buffer, 33, f);
  puts(buffer);
  fclose(f);
  return 0;
}
