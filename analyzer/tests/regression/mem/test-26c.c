// SAFE

extern int __ikos_unknown();
extern void __ikos_assert(int);
int* id(int* p) {
  return p;
}

int main() {
  int x = 5;
  int y = 7;
  int* a[1];
  int *p, *q;

  if (__ikos_unknown())
    p = id(&x);
  else
    p = id(&y);

  if (__ikos_unknown())
    q = id(&x);
  else
    q = id(&y);

  if (__ikos_unknown())
    a[0] = p; // &x;
  else
    a[0] = q; // &y;

  __ikos_assert(*(a[0]) >= 5 && *(a[0]) <= 7);

  return 42;
}
