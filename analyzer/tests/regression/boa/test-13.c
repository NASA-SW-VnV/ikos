// SAFE
// This example is to make sure that ARBOS computes struct sizes
// properly even if some types are defined recursively.

struct SimStruct_tag {
  int elem;
  struct SimStruct_tag* next;
}; // size=4+8+4(padding)=16 --> 128 bits
   // (without padding) size=4+8=12 ---> 88 bits

struct FOO_BAR {
  struct SimStruct_tag* childSfunctions; // size=8
  struct {
    struct {
      unsigned char TID[3]; // size=3
    } TaskCounters;
    double tFinal; // size=8
  } Timing;
}; // size=8+(3+8+5(padding))=24 -->  192 bits
   // (without padding) size=8+3+8=19 --->  152 bits

// v0.size   = 88         | 152          <====
// v0.offset = 0          | 0
// v1.size   = 88         | 152
// v1.offset = 88         | 64           <=====
// v2.size   = 88         | 152
// v2.offset = 88+24=112  | 64+24 = 88

#define bar1(s, val) ((s)->Timing.tFinal = (val))
#define bar2(s, idx) ((s)->Timing.TaskCounters.TID[(idx)] == 0)
#define bar3(s, idx) ((s)->Timing.TaskCounters.TID[(idx)])
struct FOO_BAR foo_M_;
struct FOO_BAR* foo_M = &foo_M_;

struct FOO_BAR* foo(void) {
  return foo_M;
}

int main(int argc, char** argv) {
  struct FOO_BAR* S;
  S = foo();
  bar1(S, 1.5);
  return 0;
}
