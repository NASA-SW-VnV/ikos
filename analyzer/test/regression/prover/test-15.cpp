extern "C" {
extern void __ikos_assert(int) noexcept;
}

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
