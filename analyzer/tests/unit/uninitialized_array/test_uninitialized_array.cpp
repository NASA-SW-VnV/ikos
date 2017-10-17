#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/domains/octagons.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/domains/uninitialized_array.hpp>

using namespace ikos;
using namespace analyzer;

typedef variable< z_number, varname_t > var_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef octagon< z_number, varname_t > scalar_domain_t;
typedef uninitialized_array_domain< scalar_domain_t, z_number, varname_t >
    uninitialized_array_domain_t;

int main(int argc, char** argv) {
  VariableFactory vfac;
  varname_t vA(vfac["a"]);
  varname_t vi(vfac["i"]);
  varname_t vn(vfac["n"]);

  // assume(n > 0)
  uninitialized_array_domain_t inv1;
  inv1.assertion(linear_constraint_t(var_t(vn) >= 1), vfac);
  inv1.set(vn, uninitialized_value::initialized());
  // i = 0;
  inv1.assign(vi, 0, vfac);
  /// loop 1st
  // i < n
  inv1.assertion(linear_constraint_t(var_t(vi) <= var_t(vn) - 1), vfac);
  // A[i] = 5;
  inv1.array_write(vA, vi, 5);
  // i++;
  inv1.apply(OP_ADDITION, vi, z_number(1), vfac);
  std::cout << "after 1st iteration: " << std::endl << inv1 << std::endl;
  // loop 2nd
  uninitialized_array_domain_t inv2(inv1);
  // i < n
  inv2.assertion(linear_constraint_t(var_t(vi) <= var_t(vn) - 1), vfac);
  // A[i] = 5;
  inv2.array_write(vA, vi, 5);
  // i++;
  inv2.apply(OP_ADDITION, vi, z_number(1), vfac);
  std::cout << "after 2st iteration: " << std::endl << inv2 << std::endl;
  // widening
  uninitialized_array_domain_t inv3((inv1 | inv2));
  std::cout << "after widening: " << std::endl << inv3 << std::endl;
  std::cout << "---------------------------" << std::endl;
}
