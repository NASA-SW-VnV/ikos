struct S {
  double d;
  unsigned char a : 2;
  unsigned char b : 3;
  int i;
};

int main() {
  struct S s;
  s.a = 2;
  s.b = 4;
}
