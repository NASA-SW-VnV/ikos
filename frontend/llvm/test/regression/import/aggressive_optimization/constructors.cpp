extern "C" {
extern void __ikos_assert(int);
}

class Vector {
public:
  int _x;
  int _y;
  int _z;

  Vector(int x, int y, int z) noexcept : _x(x), _y(y), _z(z) {}
};

int f(Vector* v) {
  return v->_y;
}

class Master {
public:
  Vector* _v;
  int* _p;

  Master() {
    _v = new Vector(1, 2, 3);
    _p = new int(4);
    __ikos_assert(f(_v) == 2);
  }
};

int main() {
  Master master;
  return 0;
}
