// UNSAFE

extern int unk();

int main() {
  int x, y;
  if (unk()) {
    y = 0;
  } else {
    y = 10;
  }

  x = 10 / y;
  return x;
}
