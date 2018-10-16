// SAFE
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a[10];
  int b[10];
  int c[10];

  int *x, *z;

  a[0] = 5;
  b[0] = 10;
  c[0] = 30;

  if (__ikos_nondet_int()) {
    x = &a[0];
    z = &c[0];
  } else {
    x = &b[0];
    z = &c[0];
  }

  *x = 20;
  *z = 50;

  __ikos_assert(a[0] >= 5 && a[0] <= 20);  // safe
  __ikos_assert(b[0] >= 10 && b[0] <= 20); // safe
  __ikos_assert(c[0] == 50);               // safe

  //  __ikos_assert ( *x == 20); // warning *x = [5,20]

  return 42;
}
