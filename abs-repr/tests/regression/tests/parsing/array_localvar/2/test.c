#include <stdio.h>
#include <string.h>

char* foo(char* a, int n) {
  int i;
  for (i = 0; i < n; i++)
    a[i] = 'A';
  return a;
}

int main() {
  char str[50];

  strcpy(str, "This is string.h library function");
  puts(str);

  memset(str, '$', 50); // SAFE
  char* A = foo(str, 10);
  // char * B; <- B will be undefined
  char B[10];
  memcpy(B, A, 10);     // SAFE
  char* C = foo(B, 10); // SAFE
  puts(C);
  return (0);
}
