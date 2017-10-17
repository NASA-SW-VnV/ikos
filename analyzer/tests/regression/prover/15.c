
extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * from Invgen test suite
 */

int main(int argc, char** argv) {
  int n;
  int i, k, j;
  n = __ikos_unknown();
  k = __ikos_unknown();
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
