extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int w = 1, z = 0, x = 0, y = 0;
  while (__ikos_unknown()) {
    while (__ikos_unknown()) {
      if (w % 2 == 1)
        x++;
      if (z % 2 == 0)
        y++;
    }
    while (__ikos_unknown()) {
      z = x + y;
      w = z + 1;
    }
  }
  __ikos_assert(x == y);
}
