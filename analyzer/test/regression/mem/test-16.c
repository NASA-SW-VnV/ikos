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
  int (*fPtr_array[2])(int);
  int x = 8;
  fPtr_array[0] = &addOne;
  if (__ikos_nondet_int()) {
    fPtr_array[1] = &addTwo;
  } else {
    fPtr_array[1] = &addThree;
  }

  int res1 = fPtr_array[0](x);
  int res2 = fPtr_array[1](x);
  __ikos_assert(res1 == 9);
  __ikos_assert(res1 >= 9 && res1 <= 11);
  __ikos_assert(res2 >= 10 && res2 <= 11);
  __ikos_assert(res2 >= 9 && res2 <= 11);
  return 42;
}
