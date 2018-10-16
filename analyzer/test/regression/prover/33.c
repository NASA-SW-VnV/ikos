extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int argc, char** argv) {
  int k = __ikos_nondet_int();
  if (k < 0)
    return 0;
  int z = k;
  int x = 0;
  int y = 0;

  while (__ikos_nondet_int()) {
    int c = 0;
    while (__ikos_nondet_int()) {
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
    while (__ikos_nondet_int()) {
      x--;
      y--;
    }
    z = k + y;
  }
  __ikos_assert(x == y);
}
