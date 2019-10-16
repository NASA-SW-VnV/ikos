extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

static int x = 0;
static int y = 0;

static int f() {
  int status = __ikos_nondet_int();
  if (status < 0) {
    return -1;
  }

  x++;

  status = __ikos_nondet_int();
  if (status < 0) {
    return -2;
  }

  x++;
  y++;
  return 0;
}

int main() {
  int status = f();
  if (status == -1) {
    __ikos_assert(x == 0 && y == 0);
  } else if (status == -2) {
    __ikos_assert(x == 1 && y == 0);
  } else {
    __ikos_assert(x == 2 && y == 1);
  }
  return 0;
}
