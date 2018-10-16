#include <stdio.h>

extern void __ikos_assert(int);

int main(int argc, char** argv) {
  // we assume argc == 4
  printf("argv[argc-1]=%s\n", argv[argc - 1]); // Last argument
  printf("argv[3]=%s\n", argv[3]);             // Last argument
  printf("argv[4]=%s\n", argv[4]);             // NULL
  printf("argv[5]=%s\n", argv[5]);             // Error
}
