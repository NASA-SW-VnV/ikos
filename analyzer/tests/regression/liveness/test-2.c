/*

Executing pass - liveness intra-procedural liveness variable analysis.
Invariant at the exit of foo:: return : _|_
Invariant at the entry  of foo:: return : {x_addr.2}
Invariant at the exit of foo:: bb6 : {x_addr.2}
Invariant at the entry  of foo:: bb6 : {x_addr.2}
Invariant at the exit of foo:: pre_bb5_bb6 : {x_addr.2}
Invariant at the entry  of foo:: pre_bb5_bb6 : {x_addr.2}
Invariant at the exit of foo:: bb5 : {x_addr.2}
Invariant at the entry  of foo:: bb5 : {x_addr.2}
Invariant at the exit of foo:: bb4 : {x_addr.2}
Invariant at the entry  of foo:: bb4 : {x_addr.1}
Invariant at the exit of foo:: pre_bb2_bb4 : {x_addr.1}
Invariant at the entry  of foo:: pre_bb2_bb4 : {x_addr.0; tmp3}
Invariant at the exit of foo:: bb3 : {x_addr.1}
Invariant at the entry  of foo:: bb3 : {x_addr.0}
Invariant at the exit of foo:: pre_bb2_bb3 : {x_addr.0}
Invariant at the entry  of foo:: pre_bb2_bb3 : {x_addr.0; tmp3}
Invariant at the exit of foo:: bb2 : {x_addr.0; tmp3}
Invariant at the entry  of foo:: bb2 : {x_addr.0}
Invariant at the exit of foo:: pre_bb_bb2 : {x_addr.0}
Invariant at the entry  of foo:: pre_bb_bb2 : {x_addr.2; tmp}
Invariant at the exit of foo:: bb1 : {x_addr.0}
Invariant at the entry  of foo:: bb1 : {x_addr.2; tmp}
Invariant at the exit of foo:: pre_bb_bb1 : {x_addr.2; tmp}
Invariant at the entry  of foo:: pre_bb_bb1 : {x_addr.2; tmp}
Invariant at the exit of foo:: bb : {x_addr.2; tmp}
Invariant at the entry  of foo:: bb : {x_addr.2}
Invariant at the exit of foo:: pre_bb5_bb : {x_addr.2}
Invariant at the entry  of foo:: pre_bb5_bb : {x_addr.2}
Invariant at the exit of foo:: entry : {x_addr.2}
Invariant at the entry  of foo:: entry : {x}
*/

int foo(int x) {
  // x = input;
  int y, z;
  while (x > 1) {
    y = x / 2;
    if (y > 3)
      x = x - y;
    z = x - 4;
    if (z > 0)
      x = x / 2;
    z = z - 1;
  }
  return x;
}
