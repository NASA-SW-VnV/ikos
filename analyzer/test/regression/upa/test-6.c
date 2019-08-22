// SAFE
// Runs some safe code with lots of special functions that are caught
// differently in the analyzer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  const char* s = "AAAA";
  int a = strlen(s);
  char* p = malloc(sizeof(*s));
  memcpy(p, s, sizeof(*s));
  printf(p);
  return a;
}
