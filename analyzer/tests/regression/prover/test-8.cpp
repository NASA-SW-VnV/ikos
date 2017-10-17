extern bool __ikos_unknown(); // does not throw exceptions
extern void __ikos_assert(bool);

int x = 0;

void may_throw() {
  x = 1;
  if (__ikos_unknown()) {
    throw nullptr;
  }
}

void do_not_throw() {
  try {
    x = 2;
    __ikos_unknown();
  } catch (void*) {
    __ikos_assert(false); // unreachable
  }
}

int main() {
  may_throw();
  do_not_throw();
}
