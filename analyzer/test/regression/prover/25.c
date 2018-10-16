extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int x = 0;
  int y = 0;
  int i = 0;
  int j = 0;

  while (__ikos_nondet_int()) {
    while (__ikos_nondet_int()) {
      if (x == y)
        i++;
      else
        j++;
    }
    if (i >= j) {
      x++;
      y++;
    } else
      y++;
  }

  __ikos_assert(i >= j);
}
