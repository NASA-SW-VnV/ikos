// SAFE
// See how we treat global variables, struct and one-dimensional arrays
// The program is safe in terms of buffer overflow although A is
// uninitialized. B is uninitialized since it is a global array.
int x = 6;

struct foo {
  int x;
};

int B[10];

int main(int argc, char** argv) {
  int A[5] = {0};

  struct foo a;
  a.x = 59;
  x++;
  B[8] = 23;
  int z = x + 7 + a.x + A[4] + B[9];
  return z;
}
