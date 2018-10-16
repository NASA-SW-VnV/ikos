int G;

class A {
public:
  virtual void f(int x) {}
  virtual int g() { return 0; }
};

class B : public A {
public:
  virtual void f(int x) { G = x; }
  virtual int g() { return 0; }
};

class C : public B {
public:
  virtual void f(int x) { G = -x; }
  virtual int g() { return 1; }
};

void h(int x) {}

int hh(int x) {
  return x * x;
}

void run(A* p) {
  int x;
  p->f(12);
  x = p->g();
  h(14);
  x = hh(15);
}

int main() {
  B b;
  C c;
  try {
    run(&b);
    run(&c);
  } catch (A& e) {
  }
  return 0;
}
