extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Based on ex16 from NECLA Static Analysis Benchmarks
 */

int main() {
  int x = __ikos_nondet_int();
  int y = __ikos_nondet_int();
  int t = y;

  if (x == y)
    return x;

  while (__ikos_nondet_int()) {
    if (x > 0)
      y = y + x;
  }

  __ikos_assert(y >= t);
}
