extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * From "Path Invariants" PLDI 07 by Beyer et al.
 */

int main() {
  int i, n, a, b;
  n = __ikos_unknown();
  if (n >= 0) { // assume( n >= 0 );
    i = 0;
    a = 0;
    b = 0;
    while (i < n) {
      if (__ikos_unknown()) {
        a = a + 1;
        b = b + 2;
      } else {
        a = a + 2;
        b = b + 1;
      }
      i = i + 1;
    }
    __ikos_assert(a + b == 3 * n);
  }
}
