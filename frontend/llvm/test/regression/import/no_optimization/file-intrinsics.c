#include <stdio.h>

int main() {
  FILE* f = fopen("/tmp/test", "rw");
  char buf[1025];
  int x;
  fgets(buf, 1024, f);
  fgetc(f);
  fputs("hello world", f);
  fprintf(f, "%d", 1);
  fscanf(f, "%d", &x);
  fflush(f);
  fclose(f);
}
