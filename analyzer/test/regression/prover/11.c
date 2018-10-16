extern void __ikos_assert(int);

/*
 * Based on ex3 from NECLA Static Analysis Benchmarks
 */

int main() {
  int j = 0;
  int i;
  int x = 100;

  for (i = 0; i < x; i++) {
    j = j + 2;
  }

  __ikos_assert(j == 2 * x);
}
