// DEFINITE UNSAFE
// See how we treat global variables, struct and one-dimensional arrays
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
  int z = x + 7 + a.x + A[5] + B[10];
  return z;
}
