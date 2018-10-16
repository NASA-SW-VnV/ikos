extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int x = 0;
  int y = 0;
  int z = 0;
  int k = 0;

  while (__ikos_nondet_int()) {
    if (k % 3 == 0)
      x++;
    y++;
    z++;
    k = x + y + z;
  }

  __ikos_assert(x == y);
  __ikos_assert(y == z);
}
