#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/constants.hpp>
#include <ikos/number/z_number.hpp>

using namespace std;
using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef constant< z_number > constant_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef variable< z_number, varname_t > z_var;
typedef constant_domain< z_number, varname_t > constant_domain_t;

int main(int argc, char** argv) {
  string_factory vfac;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t w(vfac["w"]);

    constant_domain_t inv_1, inv_2;

    // assignments
    inv_1.set(x, constant_t(z_number(2)));
    inv_1.set(y, constant_t(z_number(4)));
    inv_1.set(w, constant_t(z_number(1)));
    inv_2.set(x, constant_t(z_number(2)));
    inv_2.set(y, constant_t(z_number(4)));
    inv_2.set(w, constant_t(z_number(3)));
    cout << inv_1 << endl;
    cout << inv_2 << endl;

    // arithmetic operations
    inv_1.apply(OP_ADDITION, z, x, y);
    cout << x << " + " << y << "=" << inv_1 << endl;
    inv_2.apply(OP_ADDITION, z, x, y);
    cout << x << " + " << y << "=" << inv_2 << endl;

    // join
    inv_1 = inv_1 | inv_2;
    cout << "after join:" << inv_1 << endl;

    linear_constraint_t cst = (z_var(z) == z_var(w));
    inv_1 += cst;
    cout << "after adding " << cst << ": INV=" << inv_1 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;

  return 1;
}
