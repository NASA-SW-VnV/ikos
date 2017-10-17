#include <iostream>

#include <ikos/domains/constants.hpp>
#include <ikos/number/z_number.hpp>

using namespace std;
using namespace ikos;

int main(int argc, char** argv) {
  try {
    typedef constant< z_number > constant_t;

    // construction
    constant_t x(z_number(2));
    cout << x << endl;
    constant_t y(z_number(4));
    cout << y << endl;

    // join
    constant_t z = x | y;
    cout << z << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef constant< z_number > constant_t;

    // construction
    constant_t x(z_number(3));
    cout << x << endl;
    constant_t y(z_number(3));
    cout << y << endl;

    // join
    constant_t z = x | y;
    cout << z << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef constant< z_number > constant_t;

    // construction
    constant_t x(z_number(2));
    cout << x << endl;
    constant_t y(z_number(2));
    cout << y << endl;

    // join
    constant_t j1 = x | y;
    cout << j1 << endl;

    // construction
    constant_t a(z_number(10));
    cout << a << endl;
    constant_t b(z_number(10));
    cout << b << endl;

    // join
    constant_t j2 = a | b;
    cout << j2 << endl;

    // meet
    constant_t tmp1 = j1 & j1;
    cout << "meet of " << j1 << " and " << j1 << ": " << tmp1 << endl;

    // meet
    constant_t tmp2 = j2 & j2;
    cout << "meet of " << j2 << " and " << j2 << ": " << tmp2 << endl;

    // meet
    constant_t tmp3 = j1 & j2;
    cout << "meet of " << j1 << " and " << j2 << ": " << tmp3 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef constant< z_number > constant_t;

    // construction
    constant_t x(z_number(10));
    cout << x << endl;
    constant_t y(z_number(5));
    cout << y << endl;

    constant_t tmp1 = x + y;
    cout << x << " + " << y << "=" << tmp1 << endl;

    constant_t tmp2 = x * y;
    cout << x << " * " << y << "=" << tmp2 << endl;

    constant_t tmp3 = x / y;
    cout << x << " / " << y << "=" << tmp3 << endl;

    constant_t tmp4 = x.And(y);
    cout << x << " & " << y << "=" << tmp4 << endl;

    constant_t tmp5 = x.Or(y);
    cout << x << " | " << y << "=" << tmp5 << endl;

    constant_t tmp6 = x.Shl(4);
    cout << x << " << " << y << "=" << tmp6 << endl;

    constant_t tmp7 = x.AShr(2);
    cout << x << " >> " << y << "=" << tmp7 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;

  return 1;
}
