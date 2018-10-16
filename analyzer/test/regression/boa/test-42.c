// SAFE
extern void __ikos_assert(int);

static struct {
  int stopExecutionFlag;
  int isrOverrun;
  int overrunFlags[10];
  const char* errmsg;
} GBLbuf;

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

FOO_BAR* init() {
  return foo_M;
}

int main() {
  FOO_BAR* M = init();
  M->d[2] = 3;
  M->d[3] = 4;
  M->d[4] = M->d[2] + M->d[3];
  __ikos_assert(M->d[4] == 7);
  return 42;
}
