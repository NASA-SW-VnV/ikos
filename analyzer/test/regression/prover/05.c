extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int flag, char** argv) {
  int x = 0;
  int y = 0;

  int j = 0;
  int i = 0;

  while (__ikos_nondet_int()) {
    x++;
    y++;
    i += x;
    j += y;
    if (flag)
      j += 1;
  }
  __ikos_assert(j >= i);
}
