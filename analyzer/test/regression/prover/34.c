extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int argc, char** argv) {
  int n = __ikos_nondet_int();
  int x = 0;
  int y = 0;
  int i = 0;
  int m = 10;

  if (n < 0)
    return 0;
  while (i < n) {
    i++;
    x++;
    if (i % 2 == 0)
      y++;
  }

  if (i == m) {
    __ikos_assert(x == 2 * y);
  }
}
