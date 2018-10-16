// UNSAFE

extern int __ikos_nondet_int(void);

int main() {
  int x, y;
  if (__ikos_nondet_int()) {
    y = 0;
  } else {
    y = 10;
  }

  x = 10 / y;
  return x;
}
