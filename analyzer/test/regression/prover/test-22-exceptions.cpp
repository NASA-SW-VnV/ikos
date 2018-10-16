extern "C" {
extern int __ikos_nondet_int(void) noexcept;
extern void __ikos_assert(int) noexcept;
}

int x;

void f() {
  if (__ikos_nondet_int() == 47) {
    throw 42;
  }
}

int main() {
  x = 0;
  f();
  x = 1;

  try {
    f();
    x = 2;
  } catch (int) {
    __ikos_assert(x == 1);
  }
  __ikos_assert(x >= 1 && x <= 2);
}
