extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

// This example is extracted from "Symbolic Methods to Enhance the
// Precision of Numerical Abstract Domains" by A. Mine, (VMCAI'06).
//
// However, the technique of symbolic constant propagation 'a la' Mine
// does not help here because the copy propagation done by LLVM is
// enough.

int id(int x) {
  return x;
}

void _abs() {
  int x, y, z;
  // int a[21];
  if (__ikos_nondet_int())
    x = -10;
  else
    x = 20;
  // z=id(x);
  // y = z;
  y = x;
  if (y <= 0) {
    // y = -z;
    y = -x;
  }

  /*
    the code:

    y= x;
    if (y <= 0){
      y = -x;
    }

    is already translated by LLVM+ARBOS as

    if (x <= 0)
      y = -x;
    else
      y = x;

  */

  // return a[y]; // SAFE
  __ikos_assert(y >= 0 && y <= 20);
}

// Another example from Mine's paper.
// Again, it turns out that LLVM optimizations is enough so no need of
// "linearization" here.
void lin_1() {
  int x, y, z;
  if (__ikos_nondet_int())
    x = 5;
  else
    x = 10;

  y = 3 * x - x; // LLVM ==> y = 2*x;

  __ikos_assert(y >= 10 && y <= 20);
}

// Another example from Mine's paper.
void lin_2() {
  int t, x, y, z;
  if (__ikos_nondet_int())
    x = 5;
  else
    x = 10;
  if (__ikos_nondet_int())
    y = 2;
  else
    y = 4;
  if (__ikos_nondet_int())
    z = 6;
  else
    z = 9;

  t = (x * y) - (x * z) + z; // t = [-74,19]
  // LLVM ==> (y-z)*x +z ==> t = [-64,-1]
  __ikos_assert(t >= -74 && t <= 19);
}

int main() {
  lin_1();
  lin_2();
  _abs();
  return 42;
}
