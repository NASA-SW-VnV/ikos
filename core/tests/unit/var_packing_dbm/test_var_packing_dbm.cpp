#include <iostream>

#include <ikos/domains/var_packing_dbm.hpp>

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef variable< z_number, varname_t > z_var;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef interval< z_number > interval_t;
typedef var_packing_dbm< z_number, varname_t > domain_t;

int main(int argc, char** argv) {
  string_factory vfac;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t w(vfac["w"]);
    varname_t a(vfac["a"]);
    varname_t b(vfac["b"]);
    varname_t c(vfac["c"]);

    domain_t inv;

    std::cout << "# Test assign and forget ####################################"
              << std::endl;
    std::cout << "Initial:\n\t" << inv << std::endl;

    inv.assign(x, 42);
    std::cout << "After assignment x = 42:\n\t" << inv << std::endl;

    inv.assign(y, 43);
    std::cout << "After assignment y = 43:\n\t" << inv << std::endl;

    inv -= x;
    std::cout << "After forgetting x:\n\t" << inv << std::endl;

    inv.assign(x, z_var(y) + 1);
    std::cout << "After assignment x = y + 1:\n\t" << inv << std::endl;

    inv.assign(z, z_var(x) + 1);
    std::cout << "After assignment z = x + 1:\n\t" << inv << std::endl;

    inv -= y;
    std::cout << "After forgetting y:\n\t" << inv << std::endl;

    inv -= x;
    std::cout << "After forgetting x:\n\t" << inv << std::endl;

    inv.assign(x, z_var(z) - 38);
    std::cout << "After assignment x = z - 38:\n\t" << inv << std::endl;

    inv.assign(y, 0);
    std::cout << "After assignment y = 0:\n\t" << inv << std::endl;

    inv.assign(z, z_var(y) + 1);
    std::cout << "After assignment z = y + 1:\n\t" << inv << std::endl;

    inv.assign(z, z_var(z));
    std::cout << "After assignment z = z:\n\t" << inv << std::endl;

    inv.assign(z, z_var(z) + z_var(x));
    std::cout << "After assignment z = z + x:\n\t" << inv << std::endl;

    std::cout << "# Test apply ################################################"
              << std::endl;
    inv = domain_t::top();
    inv.assign(x, 1);
    inv.assign(y, 2);

    std::cout << "Initial:\n\t" << inv << std::endl;

    inv.apply(OP_ADDITION, z, x, z_number(1));
    std::cout << "After applying z = x + 1:\n\t" << inv << std::endl;

    inv.apply(OP_ADDITION, y, y, z);
    std::cout << "After applying y = y + z:\n\t" << inv << std::endl;

    std::cout << "# Test assignments and guards ###############################"
              << std::endl;

    inv = domain_t::top();
    inv.assign(x, 1);
    inv.assign(y, 2);
    inv.assign(z, z_var(x) + z_var(y) + 3);
    std::cout << "Assignments x = 1; y = 2; z = x + y + 3:\n\t" << inv
              << std::endl;

    inv = domain_t::top();
    inv.assign(x, 7);
    inv += z_var(y) <= 3;
    inv += z_var(y) >= 1;
    inv.assign(z, z_var(x) + 2 * z_var(y) + 1);
    std::cout << "Assignment x = 7; Guards y <= 3; y >= 1; Assignment z = x + "
                 "2*y + 1:\n\t"
              << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 7);
    inv += z_var(y) <= 3;
    inv.assign(z, z_var(x) + 2 * z_var(y) + 1);
    std::cout
        << "Assignment x = 7; Guard y <= 3; Assignment z = x + 2*y + 1:\n\t"
        << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 7);
    inv.assign(z, z_var(x) + 2 * z_var(y) + 1);
    std::cout << "Assignments x = 7; z = x + 2*y + 1:\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 1);
    inv += z_var(x) + z_var(y) >= 0;
    std::cout << "Assignment x = 1; Guard x + y >= 0:\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 1);
    inv.assign(y, 2);
    inv += z_var(x) + z_var(y) >= 5;
    std::cout << "Assigment x = 1; y = 2; Guard x + y >= 5:\n\t" << inv
              << std::endl;

    inv = domain_t::top();
    inv.assign(x, 1);
    inv += z_var(y) >= 2;
    inv += z_var(z) + z_var(y) + z_var(x) == 0;
    std::cout << "Assignment x = 1; Guard y >= 2; z + y + x == 0:\n\t" << inv
              << std::endl;

    std::cout << "# Test operator <= ##########################################"
              << std::endl;

    domain_t inv1, inv2;
    inv1.assign(x, 1);
    inv2.assign(x, 1);
    std::cout << inv1 << " <= " << inv2 << " = "
              << ((inv1 <= inv2) ? "TRUE" : "FALSE") << std::endl;

    inv1.assign(y, 2);
    std::cout << inv1 << " <= " << inv2 << " = "
              << ((inv1 <= inv2) ? "TRUE" : "FALSE") << std::endl;

    inv2.assign(z, z_var(x));
    std::cout << inv1 << " <= " << inv2 << " = "
              << ((inv1 <= inv2) ? "TRUE" : "FALSE") << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv1.assign(x, 1);
    inv1.assign(y, 2);
    inv2 += (z_var(y) - z_var(x) == 1);
    std::cout << inv1 << " <= " << inv2 << " = "
              << ((inv1 <= inv2) ? "TRUE" : "FALSE") << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv2.assign(x, 42);
    std::cout << inv1 << " <= " << inv2 << " = "
              << ((inv1 <= inv2) ? "TRUE" : "FALSE") << std::endl;

    /* tricky case */
    inv = domain_t::top();
    inv += z_var(x) - z_var(y) <= 1;
    inv -= x;
    std::cout << "{} <= " << inv << " = "
              << ((domain_t::top() <= inv) ? "TRUE" : "FALSE") << std::endl;

    std::cout << "# Test join operator ########################################"
              << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();

    inv1.assign(x, 41);
    inv2.assign(x, 43);
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    inv1 += z_var(y) <= 2;
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    inv2.assign(z, 0);
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    inv2 += z_var(y) <= 3;
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv1.assign(x, 1);
    inv1.assign(y, z_var(x) + 1);
    inv2.assign(x, 1);
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv1.assign(x, 1);
    inv2.assign(x, 1);
    inv2.assign(y, z_var(x) + 1);
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    /* tricky case */
    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv1.assign(x, 1);
    inv1 += (z_var(y) <= 10);
    inv1 += (z_var(x) + z_var(y) + z_var(z) <= 10);

    inv2.assign(y, 0);
    inv2.assign(z, 0);
    inv = inv1 | inv2;
    std::cout << inv1 << " U " << inv2 << " = " << inv << std::endl;

    std::cout << "# Test meet operator ########################################"
              << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();

    inv1.assign(x, 42);
    inv2.assign(x, 43);
    inv = inv1 & inv2;
    std::cout << inv1 << " & " << inv2 << " = " << inv << std::endl;

    inv2.assign(x, 42);
    inv = inv1 & inv2;
    std::cout << inv1 << " & " << inv2 << " = " << inv << std::endl;

    inv1 += z_var(y) <= 2;
    inv = inv1 & inv2;
    std::cout << inv1 << " & " << inv2 << " = " << inv << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();
    inv1.assign(x, 42);
    inv2.assign(y, 43);
    inv = inv1 & inv2;
    std::cout << inv1 << " & " << inv2 << " = " << inv << std::endl;

    std::cout << "# Test widening operator ####################################"
              << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();

    inv1 += z_var(x) <= 1;
    inv2 += z_var(x) <= 1;
    inv = inv1 || inv2;
    std::cout << inv1 << " || " << inv2 << " = " << inv << std::endl;

    inv2 = domain_t::top();
    inv2 += z_var(x) <= 2;
    inv = inv1 || inv2;
    std::cout << inv1 << " || " << inv2 << " = " << inv << std::endl;

    inv1.assign(z, 42);
    inv2.assign(z, 43);
    inv = inv1 || inv2;
    std::cout << inv1 << " || " << inv2 << " = " << inv << std::endl;

    std::cout << "# Test narrowing operator ###################################"
              << std::endl;

    inv1 = domain_t::top();
    inv2 = domain_t::top();

    inv2 += z_var(x) <= 1;
    inv = inv1 && inv2;
    std::cout << inv1 << " && " << inv2 << " = " << inv << std::endl;

    inv1 += z_var(x) <= 3;
    inv = inv1 && inv2;
    std::cout << inv1 << " && " << inv2 << " = " << inv << std::endl;

    std::cout << "# Test is_top method ########################################"
              << std::endl;

    inv = domain_t::top();
    std::cout << inv << " is_top = " << (inv.is_top() ? "TRUE" : "FALSE")
              << std::endl;

    inv += z_var(x) <= 1;
    std::cout << inv << " is_top = " << (inv.is_top() ? "TRUE" : "FALSE")
              << std::endl;

    inv += z_var(x) - z_var(y) <= 2;
    std::cout << inv << " is_top = " << (inv.is_top() ? "TRUE" : "FALSE")
              << std::endl;

    inv -= x;
    std::cout << inv << " is_top = " << (inv.is_top() ? "TRUE" : "FALSE")
              << std::endl;

    std::cout << "# Test interval projection ##################################"
              << std::endl;
    interval_t interval = interval_t::top();

    inv = domain_t::bottom();
    interval = inv.to_interval(x, true);
    std::cout << inv << " [x] = " << interval << std::endl;

    inv = domain_t::top();
    interval = inv.to_interval(x, true);
    std::cout << inv << " [x] = " << interval << std::endl;

    inv += z_var(x) <= 3;
    inv += z_var(x) >= 1;
    interval = inv.to_interval(x, true);
    std::cout << inv << " [x] = " << interval << std::endl;

    inv.assign(y, 42);
    interval = inv.to_interval(y, true);
    std::cout << inv << " [y] = " << interval << std::endl;

    std::cout
        << "# Test optimized operations ##################################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 0);
    std::cout << inv << std::endl;

    inv.apply(OP_ADDITION, y, x, z);
    std::cout << "\tafter y = x + z: " << inv << std::endl;

    inv.apply(OP_SUBTRACTION, y, z, x);
    std::cout << "\tafter y = z - x: " << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 1);
    std::cout << inv << std::endl;

    inv.apply(OP_MULTIPLICATION, y, x, z);
    std::cout << "\tafter y = x * z: " << inv << std::endl;

    inv.apply(OP_DIVISION, y, z, x);
    std::cout << "\tafter y = z / x: " << inv << std::endl;

  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
