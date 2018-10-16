// DEFINITE UNSAFE
#include <stdio.h>

// If the multidimensional array is global then LLVM generates a
// single GetElementPtr instruction

/* int p[2][2]; */
/* void kalman_global( void ) { */
/*   p[0][3]=1; */
/* } */

/* int main(int arg, char **argv){ */
/*   kalman_global(); */
/*   return 0; */
/* } */

// This example to show how arrays, pointer, and integers are passed
// 'by reference'
/*
Total number of checks            : 14
Total number of safe              : 7
Total number of definite unsafe   : 1
Total number of potentially unsafe: 6
*/

int id(int x) {
  return x;
}

void foo(int* rtmSampleHitPtr) {
  rtmSampleHitPtr[2] = id(3);
}

void bar(int* x) {
  *x = 5;
}

void bar2(int x[4][4]) {
  x[3][2] = 5;
}

int main(int arg, char** argv) {
  int a[5];
  int b[4][4];
  foo(a);
  int x;
  bar(&x);
  bar2(b);

  int* p;
  bar2(&p);
  printf("%d\n", a[x]);
  return 0;
}
