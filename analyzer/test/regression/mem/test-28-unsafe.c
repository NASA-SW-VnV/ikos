// DEFINITE UNSAFE
extern void __ikos_assert(int);

static struct {
  int stopExecutionFlag;
  int isrOverrun;
  int overrunFlags[10];
  const char* errmsg;
} buf;

typedef struct {
  int x;
  int y;
} foo;

typedef struct {
  int a;
  int b;
  int c;
  int d[5];
} FOO_BAR;

FOO_BAR foo_M_;
FOO_BAR* foo_M = &foo_M_;

void test2() {
  foo_M->a = 5;
  __ikos_assert(foo_M->a == 5);
  foo_M->b = 7;
  __ikos_assert(foo_M->b == 7);
  foo_M->c = foo_M->a + foo_M->b;
  __ikos_assert(foo_M->a == 5);
  __ikos_assert(foo_M->b == 7);
  __ikos_assert(foo_M->c > 12);
}

int main() {
  test2();
  return 42;
}
