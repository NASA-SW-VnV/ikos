extern void __ikos_assert(bool);
extern int __ikos_unknown();

int x;

void no_exit() {
  while (true) {
  }
}

void throw_exc() {
  throw nullptr;
}

void f() {
  if (__ikos_unknown()) {
    x = 1;
    throw_exc();
  }

  no_exit();
}

int main() {
  x = 0;
  try {
    f();
  } catch (void*) {
    __ikos_assert(x == 1);
  }
  return 0;
}
