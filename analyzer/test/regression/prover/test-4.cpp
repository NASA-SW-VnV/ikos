// test exception handling
extern "C" {
extern void __ikos_assert(int) noexcept;
extern int __ikos_nondet_int(void) noexcept;
}

int x;
int y;

int f() {
  x = 0;
  y = 0;

  try {
    if (__ikos_nondet_int()) {
      x = 1;
      throw 0x42;
    } else if (__ikos_nondet_int()) {
      return 0;
    }
  } catch (int) {
    __ikos_assert(x == 1);
    if (__ikos_nondet_int()) {
      y = 1;
      throw 0x42;
    } else {
      return 1;
    }
  }

  return 2;
}

int main() {
  try {
    int r = f();
    __ikos_assert(r >= 0 && r <= 2); // ok
    __ikos_assert(x >= 0 && x <= 1); // ok
    __ikos_assert(y == 0);           // ok
  } catch (int) {
    __ikos_assert(x == 1); // ok
    __ikos_assert(y == 1); // warning because ikos is imprecise on the type of
    // exception. The first catch() {...} catches all exceptions and rethrow
    // the exception if it does not have the type void*.
  }

  return 0;
}
