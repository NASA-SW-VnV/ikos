extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int w = 1;
  int z = 0;
  int x = 0;
  int y = 0;

  while (__ikos_nondet_int()) {
    if (w % 2 == 1) {
      x++;
      w++;
    };
    if (z % 2 == 0) {
      y++;
      z++;
    };
  }

  __ikos_assert(x <= 1);
}
