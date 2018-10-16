class Base {
private:
  int x;
  int y;
};

struct Mixin {
private:
  unsigned z;
};

struct Empty {
  void f() {}
};

class Child : public Base, public Mixin, public Empty {
private:
  int tab[10];
};

Child c;

int main(void) {
  return 0;
}
