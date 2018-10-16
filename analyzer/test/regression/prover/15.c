
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * from Invgen test suite
 */

int main(int argc, char** argv) {
  int n;
  int k, j;
  n = __ikos_nondet_int();
  k = __ikos_nondet_int();
  if (n > 0) {   // assume(n>0);
    if (k > n) { // assume(k>n);
      j = 0;
      while (j < n) {
        j++;
        k--;
      }
      __ikos_assert(k >= 0);
    }
  }
  return 0;
}
