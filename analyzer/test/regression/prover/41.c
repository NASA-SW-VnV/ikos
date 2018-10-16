extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Adapted from "Automated Error Diagnosis Using Abductive Inference" by Dillig
 * et al.
 */

int main() {
  int n = __ikos_nondet_int();
  int flag = __ikos_nondet_int();

  if (n >= 0) { //  assume(n>=0);
    int k = 1;
    if (flag) {
      k = __ikos_nondet_int();
      // assume(k>=0);
      // ??
    }
    int i = 0, j = 0;
    while (i <= n) {
      i++;
      j += i;
    }
    int z = k + i + j;
    __ikos_assert(z > 2 * n);
  }
  return 0;
}
