extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int w = 1, z = 0, x = 0, y = 0;
  while (__ikos_nondet_int()) {
    while (__ikos_nondet_int()) {
      if (w % 2 == 1)
        x++;
      if (z % 2 == 0)
        y++;
    }
    while (__ikos_nondet_int()) {
      z = x + y;
      w = z + 1;
    }
  }
  __ikos_assert(x == y);
}
