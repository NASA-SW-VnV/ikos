/*
Invariant at the exit of foo:: return : _|_
Invariant at the entry  of foo:: return : {tmp3}
Invariant at the exit of foo:: bb1 : {tmp3}
Invariant at the entry  of foo:: bb1 : {d.0}
Invariant at the exit of foo:: pre_entry_bb1 : {d.0}
Invariant at the entry  of foo:: pre_entry_bb1 : _|_
Invariant at the exit of foo:: bb : {d.0}
Invariant at the entry  of foo:: bb : _|_
Invariant at the exit of foo:: pre_entry_bb : _|_
Invariant at the entry  of foo:: pre_entry_bb : _|_
Invariant at the exit of foo:: entry : _|_
Invariant at the entry  of foo:: entry : _|_
*/
int foo() {
  int a, b, c, d, x;
  a = 3; // a, b are removed by LLVM through CP
  b = 5;
  d = 4;
  x = 100; // x is removed by LLVM through DCE
  if (a > b) {
    c = a + b;
    d = 2;
  }
  c = 4;
  return b * d + c;
}
