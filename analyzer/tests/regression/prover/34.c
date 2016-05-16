extern void __ikos_assert(int);
extern int __ikos_unknown();

int main(int n, char** argv) {
  int x = 0;
  int y = 0;
  int i = 0;
  int m = 10;

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
