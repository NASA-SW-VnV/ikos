extern void __ikos_assert(int);

/*
 * Based on "SYNERGY: A New Algorithm for Property Checking" by Gulavani et al.
 */

int main() {
  int i, c;
  i = 0;
  c = 0;
  while (i < 1000) {
    c = c + i;
    i = i + 1;
  }

  __ikos_assert(c >= 0);
}
