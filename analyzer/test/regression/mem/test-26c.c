// SAFE

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int* id(int* p) {
  return p;
}

int main() {
  int x = 5;
  int y = 7;
  int* a[1];
  int *p, *q;

  if (__ikos_nondet_int())
    p = id(&x);
  else
    p = id(&y);

  if (__ikos_nondet_int())
    q = id(&x);
  else
    q = id(&y);

  if (__ikos_nondet_int())
    a[0] = p; // &x;
  else
    a[0] = q; // &y;

  __ikos_assert(*(a[0]) >= 5 && *(a[0]) <= 7);

  return 42;
}
