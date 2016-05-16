// SAFE

// Gauge domain should help here. Basically, we need to relate the
// offsets of A and B with the loop counter. Otherwise, we get [0,+oo]
// for both offsets. With Gauge, we get something like A.offset =
// B.offset = (n-1) - \lambda and \lambda=[0,n-1]

float foo(float* p, const float* A, const float* B, int n) {
  int numNonZero = n - 1;
  while (numNonZero--) {
    *p *= (*A++) * (*B++);
  }
  return *p;
}
int main(int argc, char** argv) {
  float p;
  float A[10];
  float B[10];
  return (int)foo(&p, &A, &B, 10);
}
