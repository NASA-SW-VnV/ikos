extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int flag, char** argv) {
  int x = 1;
  int y = 1;
  int a;

  if (flag)
    a = 0;
  else
    a = 1;

  while (__ikos_nondet_int()) {
    if (flag) {
      a = x + y;
      x++;
    } else {
      a = x + y + 1;
      y++;
    }
    if (a % 2 == 1)
      y++;
    else
      x++;
  }
  // x==y

  if (flag)
    a++;
  __ikos_assert(a % 2 == 1);
}
