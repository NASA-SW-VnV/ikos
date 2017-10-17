// DEFINITE UNSAFE

extern int unk();

int main() {
  int x, y, z, w;
  z = 7;
  if (unk()) {
    y = 0;
  } else {
    y = z - 7;
  }

  // if (unk())
  // {
  x = 10 / y;
  //}

  w = 10 / (y + 1);
  return x + w;
}
