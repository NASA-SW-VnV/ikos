extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int w = 1;
  int z = 0;
  int x = 0;
  int y = 0;
  while (__ikos_unknown()) {
    if (w) {
      x++;
      w = !w;
    };
    if (!z) {
      y++;
      z = !z;
    };
  }

  __ikos_assert(x == y);
}
