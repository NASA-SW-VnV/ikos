// SAFE
extern int __ikos_unknown();
extern void __ikos_assert(int);

int main() {
  int a[3];
  a[0] = 5;
  a[1] = 10;
  int x = __ikos_unknown();
  a[x] = 7;

  __ikos_assert(a[0] >= 5 && a[0] <= 7);  // safe
  __ikos_assert(a[1] >= 7 && a[1] <= 10); // safe

  return 42;
}
