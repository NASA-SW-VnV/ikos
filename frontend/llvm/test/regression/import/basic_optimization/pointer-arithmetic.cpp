#include <stdio.h>

int f() {
  return 6;
}

unsigned char* ptr_fun = (unsigned char*)&f + 1;

const char* string_map[] = {"aaa", "bbb"};

struct vector {
  int x;
  int y;
  int z;
};

struct vector v[] = {{1, 2, 3}, {4, 5, 6}};

int* ptr = &(v[1].z);

int main() {
  return printf("%d\n", v[1].z);
}
