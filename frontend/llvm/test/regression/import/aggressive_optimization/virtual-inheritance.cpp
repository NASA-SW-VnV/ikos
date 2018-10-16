struct A {
  int x;
};

struct B {
  char* y;
};

struct C {
  bool a;
};

struct E : public virtual A, public B, public C {
  int x;

  virtual void f() {}
};

struct D : public virtual A, public virtual B, public C {
  float z;
};

struct F : public virtual A {
  int y;
};

struct G : public virtual A {
  char* z;
};

struct H : public F, public G {};

int main() {
  E e;
  D d;
  F f;
  G g;
  H h;
  return 0;
}
