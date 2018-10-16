// UNSAFE if --domain=interval-congruence
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a[3];
  a[0] = 5;
  a[1] = 10;
  // a = [ 5 | 10 | T ]
  int x = __ikos_nondet_int();
  a[x] = 7;
  // a = [ [5,7] | [7,10] | [-oo,+oo] ]

  __ikos_assert(a[0] >= 5 && a[0] <= 7);  // safe
  __ikos_assert(a[1] >= 7 && a[1] <= 10); // safe
  __ikos_assert(a[x] >= 5 && a[x] <= 10); // unsafe

  return 42;
}
