// SAFE if --domain=interval-congruence
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a[3];
  a[0] = 5;
  a[1] = 10;
  int x = __ikos_nondet_int();
  a[x] = 7;

  __ikos_assert(a[0] >= 5 && a[0] <= 7);  // safe
  __ikos_assert(a[1] >= 7 && a[1] <= 10); // safe

  return 42;
}
