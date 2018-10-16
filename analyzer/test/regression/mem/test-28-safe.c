// SAFE
extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

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

void test1() {
  buf.isrOverrun++;
  if (buf.isrOverrun > 0 /*buf.isrOverrun++*/) {
    if (__ikos_nondet_int())
      buf.stopExecutionFlag = 0;
    else
      buf.stopExecutionFlag = 1;
  }

  while (!buf.stopExecutionFlag) {
    if (__ikos_nondet_int())
      buf.stopExecutionFlag = 0;
    else
      buf.stopExecutionFlag = 1;
  }

  __ikos_assert(buf.stopExecutionFlag >= 0 && buf.stopExecutionFlag <= 1);
}

void test2() {
  foo_M->a = 5;
  __ikos_assert(foo_M->a == 5);
  foo_M->b = 7;
  __ikos_assert(foo_M->b == 7);
  foo_M->c = foo_M->a + foo_M->b;
  __ikos_assert(foo_M->a == 5);
  __ikos_assert(foo_M->b == 7);
  __ikos_assert(foo_M->c == 12);
  //  __ikos_assert (foo_M->c > 12);
}

FOO_BAR* init() {
  return foo_M;
}

void test3() {
  FOO_BAR* M = init();
  M->d[2] = 3;
  M->d[3] = 4;
  M->d[4] = M->d[2] + M->d[3];
  __ikos_assert(M->d[4] == 7);
}

int main() {
  test1(); // SAFE
  test2();
  test3(); // SAFE
  return 42;
}
