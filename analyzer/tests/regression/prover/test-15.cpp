extern void __ikos_assert(bool);

class X {
public:
  int v;

  X() : v(0) {
    v++; // constructor body
  }
};

X x;

int main() {
  __ikos_assert(x.v == 1);
}
