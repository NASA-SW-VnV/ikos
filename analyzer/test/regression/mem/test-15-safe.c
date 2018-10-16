// SAFE
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int addOne(int x) {
  return x + 1;
}

int addTwo(int x) {
  return x + 2;
}

int addThree(int x) {
  return x + 3;
}

int main() {
  int (*fPtr)(int);
  int x = 8;
  if (__ikos_nondet_int()) {
    fPtr = &addOne;
  } else if (__ikos_nondet_int()) {
    fPtr = &addTwo;
  } else {
    fPtr = &addThree;
  }

  int res = fPtr(x);
  __ikos_assert(res >= 9 && res <= 11);
  return 42;
}
