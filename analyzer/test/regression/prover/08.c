extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 *  Based on "Automatically refining abstract interpretations" fig.1
 */

int main() {
  int x = 0, y = 0;
  while (__ikos_nondet_int()) {
    if (__ikos_nondet_int()) {
      x++;
      y += 100;
    } else if (__ikos_nondet_int()) {
      if (x >= 4) {
        x++;
        y++;
      }
      if (x < 0) {
        y--;
      }
    }
  }
  __ikos_assert(x < 4 || y > 2);
}
