
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int flag, char** argv) {
  int t = 0;
  int s = 0;
  int a = 0;
  int b = 0;
  while (__ikos_nondet_int()) {
    a++;
    b++;
    s += a;
    t += b;
    if (flag) {
      t += a;
    }
  }
  // 2s >= t
  int x = 1;
  if (flag) {
    x = t - 2 * s + 2;
  }
  // x <= 2
  int y = 0;
  while (y <= x) {
    if (__ikos_nondet_int())
      y++;
    else
      y += 2;
  }
  __ikos_assert(y <= 4);
}
