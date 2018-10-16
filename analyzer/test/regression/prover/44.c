extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Adapted from ex20 from NECLA Static Analysis Benchmarks
 */

int main() {
  int k = __ikos_nondet_int();
  int flag = __ikos_nondet_int();

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
