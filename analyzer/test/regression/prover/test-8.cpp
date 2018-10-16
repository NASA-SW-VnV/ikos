extern "C" {
extern void __ikos_assert(int) noexcept;
extern int __ikos_nondet_int(void) noexcept;
}

int x = 0;

void may_throw() {
  x = 1;
  if (__ikos_nondet_int()) {
    throw 0x42;
  }
}

void does_not_throw() {
  x = 2;
}

void does_not_catch() {
  try {
    does_not_throw();
  } catch (int) {
    __ikos_assert(false); // unreachable
  }
}

int main() {
  may_throw();
  does_not_catch();
}
