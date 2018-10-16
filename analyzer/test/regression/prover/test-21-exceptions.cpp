extern "C" {
extern int __ikos_nondet_int(void) noexcept;
extern void __ikos_assert(int) noexcept;
}

int x = 0;

void may_throw() {
  x = 1;
  if (__ikos_nondet_int()) {
    throw nullptr;
  }
}

extern int f();

int main() {
  may_throw();
  try {
    __ikos_assert(x == 1);
    x = 2;
    f();
  } catch (void*) {
    __ikos_assert(x == 2);
  }
  __ikos_assert(x == 2);
}
