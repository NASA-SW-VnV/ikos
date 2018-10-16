extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int w = 1;
  int z = 0;
  int x = 0;
  int y = 0;
  while (__ikos_nondet_int()) {
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
