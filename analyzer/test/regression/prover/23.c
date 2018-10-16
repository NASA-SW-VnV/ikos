extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * ex49 from NECLA Static Analysis Benchmarks
 */

int main(int argc, char** argv) {
  int n = __ikos_nondet_int();
  int i, sum = 0;
  if (n < 0)
    return 0;
  if (n >= 0) {
    for (i = 0; i < n; ++i) {
      sum = sum + i;
    }
    __ikos_assert(sum >= 0);
  }
  return 0;
}
