// SAFE
extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int a[10];
  int* x;

  a[0] = 5;
  a[1] = 10;

  if (__ikos_unknown())
    x = &a[0];
  else
    x = &a[1];

  __ikos_assert(*x >= 5 && *x <= 10); // safe

  return 42;
}
