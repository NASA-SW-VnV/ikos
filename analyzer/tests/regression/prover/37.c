extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * Taken from "Counterexample Driven Refinement for Abstract Interpretation"
 * (TACAS'06) by Gulavani
 */

int main(int n, char** argv) {
  int x = 0;
  int m = 0;
  while (x < n) {
    if (__ikos_unknown()) {
      m = x;
    }
    x = x + 1;
  }
  if (n > 0) {
    __ikos_assert(0 <= m && m < n);
  }
}
