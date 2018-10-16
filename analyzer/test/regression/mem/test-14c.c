// SAFE
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a[10];
  int* x;

  a[0] = 5;
  a[1] = 10;

  if (__ikos_nondet_int())
    x = &a[0];
  else
    x = &a[1];

  __ikos_assert(*x >= 5 && *x <= 10); // safe

  return 42;
}
