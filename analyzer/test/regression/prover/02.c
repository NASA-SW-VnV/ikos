extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int i = 1;
  int j = 0;
  int z = i - j;
  int x = 0;
  int y = 0;
  int w = 0;

  while (__ikos_nondet_int()) {
    z += x + y + w;
    y++;
    if (z % 2 == 1)
      x++;
    w += 2;
  }

  __ikos_assert(x == y);
}
