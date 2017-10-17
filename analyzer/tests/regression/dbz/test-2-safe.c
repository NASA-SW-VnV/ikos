// SAFE

extern int unk();

// c2air test-2.c && arbos -load=DBZ.so -dbz test-2.air
int main() {
  int x, y;
  if (unk()) {
    y = 3;
  } else {
    y = 10;
  }

  x = 10 / y;
  return x;
}
