#include <iostream>

#include <ikos/common/bignums.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/algorithms/linear_constraints.hpp>

#include <ikos/domains/nullity.hpp>

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef variable< z_number, varname_t > z_var;
typedef nullity_domain< varname_t > nullity_domain_t;

typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef interval_domain< z_number, varname_t > interval_domain_t;

int main(int argc, char** argv) {
  string_factory vfac;

  try {
    varname_t x1(vfac["x1"]);
    varname_t y1(vfac["y1"]);
    varname_t x2(vfac["x2"]);
    varname_t y2(vfac["y2"]);

    nullity_domain_t inv1;
    inv1.set(x2, nullity_value::non_null());
    inv1.set(y2, nullity_value::non_null());

    nullity_domain_t inv2;

    inv2.set(x2, nullity_value::non_null());
    inv2.set(y2, nullity_value::null());

    nullity_domain_t inv3 = (inv1 | inv2);
    std::cout << inv1 << " | " << inv2 << "=" << inv3 << std::endl;
    std::cout << "---------------------------" << std::endl;
  } catch (ikos_error& e) {
    std::cout << e;
  }
}
