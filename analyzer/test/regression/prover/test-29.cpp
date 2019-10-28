extern "C" {
extern void __ikos_assert(int) noexcept;
extern int __ikos_nondet_int(void) noexcept;
}

static int x = 0;
static int y = 0;
static int z = 0;

static int f() {
  int status = __ikos_nondet_int();
  if (status < 0) {
    throw nullptr;
  }

  x++;

  status = __ikos_nondet_int();
  if (status < 0) {
    return -1;
  }

  x++;
  y++;

  status = __ikos_nondet_int();
  if (status < 0) {
    return -2;
  }

  x++;
  y++;
  z++;
  return 0;
}

int main() {
  try {
    int status = f();

    if (status == -1) {
      __ikos_assert(x == 1 && y == 0 && z == 0);
    } else if (status == -2) {
      __ikos_assert(x == 2 && y == 1 && z == 0);
    } else {
      __ikos_assert(x == 3 && y == 2 && z == 1);
    }
  } catch (...) {
    __ikos_assert(x == 0 && y == 0 && z == 0);
  }
  return 0;
}
