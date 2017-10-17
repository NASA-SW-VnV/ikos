#include <stdio.h>
void foo(int n) {
  int a[n], i;
  for (i = 0; i < n; i++) {
    a[i] = i * i;
  }
  a[n] = n * n;
}

int main(int argc, char** argv) {
  int v;
  scanf("%d", &v);
  foo(v);
  return 0;
}
