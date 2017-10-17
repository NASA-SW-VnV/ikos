#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/nullity.hpp>

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef nullity_domain_impl< varname_t > nullity_domain_t;

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
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
