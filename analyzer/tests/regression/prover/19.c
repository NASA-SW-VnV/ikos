extern void __ikos_assert(int);
extern int __ikos_unknown();
/*
 * From "Simplifying Loop Invariant Generation using Splitter Predicates",
 * Sharma et al. CAV'11
 */

int main() {
  int n = __ikos_unknown();
  int m = __ikos_unknown();

  if (n >= 0) {    // assume(n>=0);
    if (m >= 0) {  // assume(m>=0);
      if (m < n) { // assume(m<n);
        int x = 0;
        int y = m;
        while (x < n) {
          x++;
          if (x > m)
            y++;
        }
        __ikos_assert(y == n);
      }
    }
  }
}
