extern void __ikos_assert(int);
extern int __ikos_unknown();

int main(int argc, char** argv) {
  int k = __ikos_unknown();
  if (k < 0)
    return 0;
  int z = k;
  int x = 0;
  int y = 0;

  while (__ikos_unknown()) {
    int c = 0;
    while (__ikos_unknown()) {
      if (z == k + y - c) {
        x++;
        y++;
        c++;
      } else {
        x++;
        y--;
        c++;
      }
    }
    while (__ikos_unknown()) {
      x--;
      y--;
    }
    z = k + y;
  }
  __ikos_assert(x == y);
}
