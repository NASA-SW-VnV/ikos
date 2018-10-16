// DEFINITE UNSAFE

extern int __ikos_nondet_int(void);

int main() {
  int x, y, z, w;
  z = 7;
  if (__ikos_nondet_int()) {
    y = 0;
  } else {
    y = z - 7;
  }

  // if (__ikos_nondet_int())
  // {
  x = 10 / y;
  //}

  w = 10 / (y + 1);
  return x + w;
}
