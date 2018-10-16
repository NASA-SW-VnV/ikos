extern "C" {
extern void __ikos_assert(int) noexcept;
extern int __ikos_nondet_int(void) noexcept;
}

int x;

void no_exit() {
  while (true) {
  }
}

void throw_exc() {
  throw 0x42;
}

void f() {
  if (__ikos_nondet_int()) {
    x = 1;
    throw_exc();
  }

  no_exit();
}

int main() {
  x = 0;
  try {
    f();
  } catch (int) {
    __ikos_assert(x == 1);
  }
  return 0;
}
