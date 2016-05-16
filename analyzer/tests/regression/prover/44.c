extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * Adapted from ex20 from NECLA Static Analysis Benchmarks
 */

int main() {
  int k = __ikos_unknown();
  int flag = __ikos_unknown();

  int i = 0;
  int j = 0;
  int n;

  if (flag == 1) {
    n = 1;
  } else {
    n = 2;
  }

  i = 0;

  while (i <= k) {
    i++;
    j = j + n;
  }
  if (flag == 1) {
    __ikos_assert(j == i);
  }
}
