extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Taken from "Counterexample Driven Refinement for Abstract Interpretation"
 * (TACAS'06) by Gulavani
 */

int main(int argc, char** argv) {
  int n = __ikos_nondet_int();
  int x = 0;
  int m = 0;
  if (n < 0)
    return 0;
  while (x < n) {
    if (__ikos_nondet_int()) {
      m = x;
    }
    x = x + 1;
  }
  if (n > 0) {
    __ikos_assert(0 <= m && m < n);
  }
}
