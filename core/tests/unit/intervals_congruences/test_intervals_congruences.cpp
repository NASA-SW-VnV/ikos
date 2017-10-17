#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/intervals_congruences.hpp>
#include <ikos/number/z_number.hpp>

using namespace std;
using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef congruence< z_number > congruence_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef variable< z_number, varname_t > z_var;
typedef interval_congruence_domain< z_number, varname_t >
    interval_congruence_domain_t;

int main(int argc, char** argv) {
  string_factory vfac;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t w(vfac["w"]);

    interval_congruence_domain_t inv_1, inv_2;

    // assignments: no "set" method in reduced products
    inv_1.assign(x, linear_expression_t(z_number(2)));
    inv_1.assign(y, linear_expression_t(z_number(4)));
    inv_1.assign(w, linear_expression_t(z_number(1)));
    inv_2.assign(x, linear_expression_t(z_number(0)));
    inv_2.assign(y, linear_expression_t(z_number(2)));
    inv_2.assign(w, linear_expression_t(z_number(3)));
    // arithmetic operations
    inv_1.apply(OP_ADDITION, z, x, y);
    inv_2.apply(OP_ADDITION, z, x, y);

    cout << inv_1 << endl;
    cout << inv_2 << endl;

    // // join
    inv_1 = inv_1 | inv_2;
    cout << "after join:" << inv_1 << endl;

    // linear_constraint_t cst = (z_var(z) == z_var(w));
    // inv_1 += cst;
    // cout << "after adding " << cst <<  ": INV=" << inv_1 << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t w(vfac["w"]);

    interval_congruence_domain_t inv;

    inv.apply(OP_MULTIPLICATION, x, y, z_number(4));
    cout << inv << endl;
    inv += (z_var(x) <= 7);
    cout << "after x <= 7: " << inv << endl;
    inv += (z_var(x) >= 1);
    cout << "after x >= 1: " << inv << endl;

    inv = interval_congruence_domain_t::top();
    inv.apply(OP_MULTIPLICATION, x, y, z_number(4));
    cout << inv << endl;
    inv += (z_var(x) <= 3);
    cout << "after x <= 3: " << inv << endl;
    inv += (z_var(x) >= 1);
    cout << "after x >= 1: " << inv << endl;

    inv = interval_congruence_domain_t::top();
    inv.apply(OP_MULTIPLICATION, x, y, z_number(4));
    inv.apply(OP_ADDITION, x, x, z_number(2));
    cout << inv << endl;
    inv += (z_var(x) >= 2);
    cout << "after x >= 2: " << inv << endl;
    inv += (z_var(x) <= 3);
    cout << "after x <= 3: " << inv << endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  cout << "---------------------------" << endl;

  return 1;
}
