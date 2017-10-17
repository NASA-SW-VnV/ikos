extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int x = 0;
  int y = 0;
  int i = 0;
  int j = 0;

  while (__ikos_unknown()) {
    while (__ikos_unknown()) {
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
