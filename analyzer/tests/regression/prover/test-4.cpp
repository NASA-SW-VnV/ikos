// test exception handling

int x;
int y;

extern void __ikos_assert(bool);
extern int __ikos_unknown();

int f() {
  x = 0;
  y = 0;

  try {
    if (__ikos_unknown()) {
      x = 1;
      throw nullptr;
    } else if (__ikos_unknown()) {
      return 0;
    }
  } catch (void*) {
    __ikos_assert(x == 1);
    if (__ikos_unknown()) {
      y = 1;
      throw nullptr;
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
  } catch (void*) {
    __ikos_assert(x == 1); // ok
    __ikos_assert(y == 1); // warning because ikos is imprecise on the type of
    // exception. The first catch() {...} catches all exceptions and rethrow
    // the exception if it does not have the type void*.
  }

  return 0;
}
