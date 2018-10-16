extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int i = 0;
  int x = 2;
  while (__ikos_nondet_int()) {
    x = 3;
    i++;
  }
  __ikos_assert(i >= 0); // any numerical domain >= intervals will do it
  __ikos_assert(x == 2 || x == 3); // a numerical domain with some
                                   // reasoning for disequalities (currently
  // only intervals if the interval is a singleton)
  return 0;
}
