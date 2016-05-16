extern void __ikos_assert(int);
extern int __ikos_unknown();

#define TRUE 1

int main() {
  int s = 0;
  while (TRUE) {
    if (__ikos_unknown())
      break;
    if (s < 5)
      s++;
    else
      s = 0;
  }
  __ikos_assert(s >= 0 && s <= 5);

  return 42;
}
