extern "C" {
extern void __ikos_assert(int) noexcept;
}

class A {
public:
  A() noexcept {}
  virtual void f() { __ikos_assert(false); }
  virtual void g() { __ikos_assert(false); }
  virtual ~A() {}
};

class B : public A {
public:
  B() noexcept {}
  void f() { __ikos_assert(true); }
  void g() { __ikos_assert(false); }
};

class C : public A {
public:
  C() noexcept {}
  void f() { __ikos_assert(false); }
  void g() { __ikos_assert(false); }
};

int main() {
  A* a = new B();
  a->f();
  return 0;
}
