// for relational domains
extern void __ikos_assert(int);

int main() {
  int i = 0;
  int x = 0;
  while (i < 10) {
    i++;
    x += 32;
  }

  __ikos_assert(x == 320);
  return 0;
}
