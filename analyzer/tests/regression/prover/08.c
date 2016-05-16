extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 *  Based on "Automatically refining abstract interpretations" fig.1
 */

int main() {
  int x = 0, y = 0;
  while (__ikos_unknown()) {
    if (__ikos_unknown()) {
      x++;
      y += 100;
    } else if (__ikos_unknown()) {
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
