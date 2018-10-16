class Foo {};

int main() {
  Foo* a = new Foo();
  Foo* b = a;
  delete a;
  delete b;
}
