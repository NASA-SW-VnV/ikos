// SAFE
extern void __ikos_assert(int);

typedef struct {
  int* sampleHits;
  int y[5];
} foo;

struct FOO_BAR {
  int a;
  int b;
  int c;
  foo d[5];
  struct {
    int clockTick;
    struct {
      char TID0_2;
      char TID1_2;
    } RateInteraction;
    int* sampleHits;
  } Timing;
};

#ifndef bar1
#define bar1(bar) ((bar)->Timing.sampleHits)
#endif

struct FOO_BAR foo_M_;
struct FOO_BAR* foo_M = &foo_M_;
int sampleHit = 20;

static struct FOO_BAR* init() {
  struct FOO_BAR* tmp = foo_M;
  bar1(tmp) = &sampleHit;
  return tmp;
}

static void oneStep(struct FOO_BAR* M) {
  int* p = bar1(M);
  M->d[2].y[0] = 3;
  M->d[3].y[0] = 4;
  M->d[4].y[0] = M->d[2].y[0] + M->d[3].y[0] + *p;
  // __ikos_assert (M->d[4].y[0] == 27);
}
int main() {
  struct FOO_BAR* M = init();
  oneStep(M);
  return 42;
}
