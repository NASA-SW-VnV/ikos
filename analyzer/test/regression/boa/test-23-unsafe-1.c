// DEFINITE UNSAFE
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

  memset(str, '$', 100); // DEFINITE ERROR
  char* A = foo(str, 7);
  // char * B; <- B will be undefined
  char B[50];
  memcpy(B, A, 5);
  char* C = foo(B, 6);
  puts(C);
  return (0);
}
