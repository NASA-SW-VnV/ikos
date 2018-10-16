// SAFE if --domain=interval-congruence
int main(int argc, char** argv) {
  int k = 0;
  int i = 0;
  int j = 1;
  int A[10];
  for (k = 0; k < 10; k++) {
    A[k] = 0;
    i = i + 2;
    j = j + 2;
  }
  if (i % 2 == 1) {
    A[k] = 1; // unsafe but dead code
  }
  return i + j;
}
