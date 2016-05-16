extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * ex49 from NECLA Static Analysis Benchmarks
 */

int main(int n, char** argv) {
  int i, sum = 0;
  // assume( n >= 0);
  if (n >= 0) {
    for (i = 0; i < n; ++i) {
      sum = sum + i;
    }
    __ikos_assert(sum >= 0);
  }
  return 0;
}
