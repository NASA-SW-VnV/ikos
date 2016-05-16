extern void __ikos_assert(int);
extern int __ikos_unknown();
/*
 * Based on ex16 from NECLA Static Analysis Benchmarks
 */

int main() {
  int x = __ikos_unknown();
  int y = __ikos_unknown();

  int i = 0;
  int t = y;

  if (x == y)
    return x;

  while (__ikos_unknown()) {
    if (x > 0)
      y = y + x;
  }

  __ikos_assert(y >= t);
}
