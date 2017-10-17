// SAFE

extern int __ikos_unknown();
extern void __ikos_assert(int);

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
  if (__ikos_unknown())
    p = &x;
  else
    p = &y;

  if (__ikos_unknown())
    q = fst(&x, &y);
  else
    q = snd(&x, &y);

  __ikos_assert(*p >= 5 && *p <= 7);
  __ikos_assert(*q >= 5 && *q <= 7);

  return 42;
}
