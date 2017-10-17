extern void __ikos_assert(int);
extern int __ikos_unknown();

int main(int flag, char** argv) {
  int x = 0;
  int y = 0;
  int j = 0;
  int i = 0;
  int c = 0;
  int d = 1;
  while (__ikos_unknown()) {
    x++;
    y++;
    i += x;
    j += y;
    if (flag) {
      j += 1;
    }
  }
  if (j >= i)
    x = y;
  else
    x = y + 1;

  int w = 1;
  int z = 0;
  while (__ikos_unknown()) {
    while (__ikos_unknown()) {
      if (w % 2 == 1)
        x++;
      if (z % 2 == 0)
        y++;
    }
    z = x + y;
    w = z + 1;
  }
  __ikos_assert(x == y);
}
