/*

This is a bit hard to visualize. d_XXX represents the statement UID
XXX. Thus, to check if the results makes sense you need to print first
the program with its statements UID's.

Executing pass - reaching intra-procedural reaching definition analysis.
Invariant at the entry of foo:: entry : _|_
Invariant at the exit  of foo:: entry : {d_179}
Invariant at the entry of foo:: bb5 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb5 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: pre_bb5_bb : {d_187; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb5_bb : {d_187; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: bb : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: pre_bb_bb1 : {d_187; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb_bb1 : {d_187; d_176; d_178; d_172; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: bb1 : {d_187; d_176; d_178; d_172; d_165; d_159;
d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb1 : {d_187; d_176; d_178; d_172; d_159; d_180;
d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: pre_bb_bb2 : {d_187; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb_bb2 : {d_187; d_176; d_178; d_172; d_163;
d_165; d_180; d_74; d_177; d_53; d_34; d_12; d_179}
Invariant at the entry of foo:: bb2 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb2 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: pre_bb2_bb4 : {d_187; d_176; d_178; d_172;
d_163; d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb2_bb4 : {d_187; d_176; d_178; d_163;
d_165; d_159; d_180; d_74; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: pre_bb2_bb3 : {d_187; d_176; d_178; d_172;
d_163; d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb2_bb3 : {d_187; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: bb3 : {d_187; d_178; d_172; d_163; d_165; d_159;
d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb3 : {d_187; d_172; d_163; d_165; d_159; d_180;
d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: bb4 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb4 : {d_187; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12}
Invariant at the entry of foo:: pre_bb5_bb6 : {d_187; d_176; d_178; d_172;
d_163; d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: pre_bb5_bb6 : {d_191; d_176; d_178; d_172;
d_163; d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: bb6 : {d_191; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: bb6 : {d_191; d_176; d_178; d_172; d_163; d_165;
d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of foo:: return : {d_191; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the exit  of foo:: return : {d_191; d_176; d_178; d_172; d_163;
d_165; d_159; d_180; d_74; d_177; d_53; d_34; d_164; d_12; d_179}
Invariant at the entry of main:: entry : _|_
Invariant at the exit  of main:: entry : {d_136}
Invariant at the entry of main:: return : {d_136}
Invariant at the exit  of main:: return : {d_136}
*/
int foo(int N) {
  int x, y, z;
  x = N;

  while (x > 1) {
    y = x / 2;
    if (y > 3)
      x = x - y;
    z = x - 4;
    if (z > 0)
      x = x - 2;
    z = z - 1;
  }
  return x;
}

int main(int argc, char** argv) {
  return foo(100);
}
