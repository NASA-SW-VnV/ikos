class Foo {};

int main() {
  class Foo* a = new Foo[42];
  class Foo* b = a;

  delete[] a;
  delete[] b;
}
