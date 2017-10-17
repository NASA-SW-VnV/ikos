#include <iostream>

#include <ikos/domains/congruences.hpp>
#include <ikos/number/z_number.hpp>

using namespace std;
using namespace ikos;

int main(int argc, char** argv) {
  try {
    typedef congruence< z_number > congruence_t;

    // construction
    congruence_t x(z_number(2));
    cout << x << endl;
    congruence_t y(z_number(4));
    cout << y << endl;

    // join
    congruence_t z = x | y;
    cout << z << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef congruence< z_number > congruence_t;

    // construction
    congruence_t x(z_number(1));
    cout << x << endl;
    congruence_t y(z_number(3));
    cout << y << endl;

    // join
    congruence_t z = x | y;
    cout << z << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef congruence< z_number > congruence_t;

    // construction
    congruence_t x(z_number(0));
    cout << x << endl;
    congruence_t y(z_number(2));
    cout << y << endl;

    // join
    congruence_t even = x | y;
    cout << even << endl;

    // construction
    congruence_t a(z_number(1));
    cout << a << endl;
    congruence_t b(z_number(3));
    cout << b << endl;

    // join
    congruence_t odd = a | b;
    cout << odd << endl;

    // meet of even and 1 should be bottom
    congruence_t tmp1 = even & a;
    cout << "meet of even and 1: " << tmp1 << endl;

    // meet of even and 1 should be 1
    congruence_t tmp2 = odd & a;
    cout << "meet of odd and 1: " << tmp2 << endl;

    // meet of even and odd should be bottom
    congruence_t tmp3 = even & odd;
    cout << "meet of even and odd numbers: " << tmp3 << endl;

    // join of even and odd should be Z
    congruence_t tmp4 = even | odd;
    cout << "join of even and odd numbers: " << tmp4 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  cout << "---------------------------" << endl;

  try {
    typedef congruence< z_number > congruence_t;

    // construction
    congruence_t x(z_number(0));
    cout << x << endl;
    congruence_t y(z_number(2));
    cout << y << endl;

    // join
    congruence_t z = x | y;
    cout << z << endl;

    congruence_t b(z_number(1));
    congruence_t tmp1 = z + b;
    cout << "(" << z << ") + (" << b << ") = " << tmp1 << endl;

    congruence_t tmp2 = z * y;
    cout << "(" << z << ") * (" << y << ") = " << tmp2 << endl;

    congruence_t tmp3 = z % y;
    cout << "(" << z << ") % (" << y << ") = " << tmp3 << endl;

    congruence_t tmp4 = z / y;
    cout << "(" << z << ") / (" << y << ") = " << tmp4 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;

  try {
    typedef congruence< z_number > congruence_t;

    // construction
    congruence_t x = congruence_t::top();
    cout << x << endl;
    congruence_t y(z_number(4));
    cout << y << endl;

    // join
    congruence_t z = x | y;
    cout << z << endl;

    congruence_t tmp1 = x + y;
    cout << "(" << x << ") + (" << y << ") = " << tmp1 << endl;

    congruence_t tmp2 = x * y;
    cout << "(" << x << ") * (" << y << ") = " << tmp2 << endl;

    congruence_t tmp3 = x % y;
    cout << "(" << x << ") % (" << y << ") = " << tmp3 << endl;

    congruence_t tmp4 = x / y;
    cout << "(" << x << ") / (" << y << ") = " << tmp4 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;
  return 1;
}
