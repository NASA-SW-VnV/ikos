/*
Invariant at the exit of foo:: return : _|_
Invariant at the entry of foo:: return : {tmp3}
Invariant at the exit of foo:: bb2 : {tmp3}
Invariant at the entry of foo:: bb2 : {ret.0}
Invariant at the exit of foo:: pre_bb1_bb2 : {ret.0}
Invariant at the entry of foo:: pre_bb1_bb2 : {i.0; ret.0; b}
Invariant at the exit of foo:: bb1 : {i.0; ret.0; a; b}
Invariant at the entry of foo:: bb1 : {i.0; ret.0; a; b}
Invariant at the exit of foo:: bb : {i.0; ret.0; a; b}
Invariant at the entry of foo:: bb : {i.0; ret.0; a; b}
Invariant at the exit of foo:: pre_bb1_bb : {i.0; ret.0; a; b}
Invariant at the entry of foo:: pre_bb1_bb : {i.0; ret.0; a; b}
Invariant at the exit of foo:: entry : {i.0; ret.0; a; b}
Invariant at the entry of foo:: entry : {a; b}
*/
int foo(int a, int b) {
  // {a,b}
  int i = 0;
  // {i,a,b}
  int g = 0;
  // {i,g,a,b}
  int x = g;
  // {i,x,a,b}
  int ret = 0;
  // {i,ret,x,a,b}
  while (i < b) {
    // {i,ret,x,a}
    ret = ret * a;
    // {i,ret,x}
    i = i + 1;
  }
  //{ret,x}
  return ret + x;
}
