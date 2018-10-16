// SAFE
extern void __ikos_assert(int);

int main() {
  int a[10];
  int b[10];

  int* x = &a[0];
  int* y = &b[0];

  x = x + 2;
  *x = 3;
  x++;
  x--;
  y = y + 4;
  *y = 19;

  y = x;
  *y = 5;
  __ikos_assert(*x == 5);
  return 42;
}
