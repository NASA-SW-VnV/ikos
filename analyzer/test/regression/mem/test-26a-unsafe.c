// UNSAFE

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int* fst(int* a, int* b) {
  return a;
}

int* snd(int* a, int* b) {
  return b;
}

int main() {
  int x = 5;
  int y = 7;
  int *p, *q;
  if (__ikos_nondet_int())
    p = &x;
  else
    p = &y;

  if (__ikos_nondet_int())
    q = fst(&x, &y);
  else
    q = snd(&x, &y);

  __ikos_assert(*p == 5);
  __ikos_assert(*q == 7);

  return 42;
}
