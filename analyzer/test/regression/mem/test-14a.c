// SAFE
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a[10];
  int b[10];
  int* x;

  a[0] = 5;
  b[0] = 10;

  if (__ikos_nondet_int())
    x = &a[0];
  else
    x = &b[0];

  //__ikos_assert ( *x == 5);             // warning
  //__ikos_assert ( *x == 10);            // warning
  __ikos_assert(*x >= 5 && *x <= 10); // safe
  return 42;
}
