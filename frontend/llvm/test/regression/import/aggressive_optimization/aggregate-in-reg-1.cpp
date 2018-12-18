template < typename T >
class Vector3 {
private:
  T _x, _y, _z;

public:
  Vector3< T >(T x, T y, T z) : _x(x), _y(y), _z(z) {}
};

class Foo {
private:
  Vector3< float > coord;

public:
  Foo(float x, float y, float z) : coord(Vector3< float >(x, y, z)) {}
  Vector3< float > get_coord() { return coord; }
};

int main(int argc, char* argv[]) {
  Foo f(1, 2, 3);
  Vector3< float > coord = f.get_coord();
  return 0;
}
