#include <iostream>

#include <ikos/domains/intervals.hpp>
#include <ikos/domains/union_domains.hpp>
#include <ikos/number/z_number.hpp>

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef variable< z_number, varname_t > z_var;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef interval< z_number > interval_t;
typedef bound< z_number > bound_t;

typedef interval_domain< z_number, varname_t > interval_domain_t;
typedef union_domain< z_number, varname_t, interval_domain_t, 2 > domain_t;

int main(int argc, char** argv) {
  string_factory vfac;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t a(vfac["a"]);
    varname_t b(vfac["b"]);
    varname_t c(vfac["c"]);

    std::cout
        << "# Test is_top() and is_bottom() ##############################"
        << std::endl;

    domain_t inv = domain_t::top();
    std::cout << "Top:\n\t" << inv << std::endl;
    std::cout << "is_bottom = " << inv.is_bottom() << std::endl;
    std::cout << "is_top = " << inv.is_top() << std::endl;

    inv = domain_t::bottom();
    std::cout << "Bottom:\n\t" << inv << std::endl;
    std::cout << "is_bottom = " << inv.is_bottom() << std::endl;
    std::cout << "is_top = " << inv.is_top() << std::endl;

    std::cout
        << "# Test assign() and is_bottom() ##############################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 42);
    std::cout << "Top:" << std::endl
              << "Assign(x, 42):" << std::endl
              << "\t" << inv << std::endl;
    std::cout << "is_bottom = " << inv.is_bottom() << std::endl;
    std::cout << "is_top = " << inv.is_top() << std::endl;

    std::cout
        << "# Test join ##################################################"
        << std::endl;

    domain_t inv_a = domain_t::top();
    domain_t inv_b = domain_t::top();
    inv_a.assign(x, 0);
    inv_b.assign(x, 2);
    inv = inv_a | inv_b;
    std::cout << "{x = 0} U {x = 2}:" << std::endl << "\t" << inv << std::endl;
    std::cout << "is_bottom = " << inv.is_bottom() << std::endl;
    std::cout << "is_top = " << inv.is_top() << std::endl;

    inv_a = domain_t::top();
    inv_a.assign(x, 4);
    inv = inv | inv_a;
    std::cout << "({x = 0} U {x = 2}) U {x = 4}:" << std::endl
              << "\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_a.assign(x, 6);
    inv_b = domain_t::top();
    inv_b.assign(x, 8);
    inv_a = inv_a | inv_b;

    inv_b = domain_t::top();
    inv_b.assign(x, 10);

    inv_a = inv_b | inv_a;
    inv = inv | inv_a;
    std::cout
        << "(({x = 0} U {x = 2}) U {x = 4}) U ({x = 10} U ({x = 6} U {x = 8})):"
        << std::endl
        << "\t" << inv << std::endl;

    std::cout
        << "# Test operator+=() ##########################################"
        << std::endl;

    inv += (z_var(x) >= 5);
    std::cout << "operator+=(x >= 5):\n\t" << inv << std::endl;

    inv += (z_var(x) <= 7);
    std::cout << "operator+=(x <= 7):\n\t" << inv << std::endl;

    inv += (z_var(x) >= 8);
    std::cout << "operator+=(x >= 8):\n\t" << inv << std::endl;

    std::cout
        << "# Test set() #################################################"
        << std::endl;

    inv_a = domain_t::top();
    inv_a.assign(a, 0);
    inv_b.assign(a, 2);
    inv = inv_a | inv_b;
    inv.set(x, interval_t(0, 4));
    inv.set(y, interval_t(bound_t::minus_infinity(), 42));
    inv.set(z, interval_t(42, bound_t::plus_infinity()));
    std::cout << "{a = 0} U {a = 2}:" << std::endl
              << "set(x, [0, 4]):" << std::endl
              << "set(y, ]-oo, 42]):" << std::endl
              << "set(z, [42, +oo[):" << std::endl
              << "\t" << inv << std::endl;

    std::cout
        << "# Test operator[]() ##########################################"
        << std::endl;

    std::cout << "inv[x]: " << inv[x] << std::endl;
    std::cout << "inv[y]: " << inv[y] << std::endl;
    std::cout << "inv[z]: " << inv[z] << std::endl;
    std::cout << "inv[a]: " << inv[a] << std::endl;

    std::cout
        << "# Test operator<=() ##########################################"
        << std::endl;

    std::cout << "top() <= bottom():\n\t"
              << (domain_t::top() <= domain_t::bottom()) << std::endl;
    std::cout << "bottom() <= top():\n\t"
              << (domain_t::bottom() <= domain_t::top()) << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.assign(x, 1);
    std::cout << "{x = 1} <= top():\n\t" << (inv_a <= inv_b) << std::endl;

    inv_b += (z_var(x) <= 1);
    std::cout << "{x = 1} <= {x <= 1}:\n\t" << (inv_a <= inv_b) << std::endl;

    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 0);
    std::cout << "{x = 1} <= {x <= 0}:\n\t" << (inv_a <= inv_b) << std::endl;

    inv_a.assign(y, 2);
    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 1);
    std::cout << "{x = 1, y = 2} <= {x <= 1}:\n\t" << (inv_a <= inv_b)
              << std::endl;

    inv_b += (z_var(z) <= 4);
    std::cout << "{x = 1, y = 2} <= {x <= 1, z <= 4}:\n\t" << (inv_a <= inv_b)
              << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();

    inv_a.assign(x, 1);
    inv_a += (z_var(y) <= 2);
    inv_a.assign(z, 3);
    inv_a += (z_var(a) >= 4);
    inv_a.assign(b, 5);

    inv_b += (z_var(y) <= 3);
    inv_b += (z_var(a) >= 1);
    inv_b.assign(z, 3);
    inv_b.set(x, interval_t(-1, 1));

    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} <= {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 1}:\n\t"
              << (inv_a <= inv_b) << std::endl;

    inv_b += (z_var(a) >= 5);
    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} <= {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 5}:\n\t"
              << (inv_a <= inv_b) << std::endl;

    std::cout
        << "# Test operator&() ###########################################"
        << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::bottom();
    inv = inv_a & inv_b;
    std::cout << "top() & bottom():\n\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.assign(x, 1);
    inv = inv_a & inv_b;
    std::cout << "{x = 1} & top():\n\t" << inv << std::endl;

    inv_b += (z_var(x) <= 1);
    inv = inv_a & inv_b;
    std::cout << "{x = 1} & {x <= 1}:\n\t" << inv << std::endl;

    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 0);
    inv = inv_a & inv_b;
    std::cout << "{x = 1} & {x <= 0}:\n\t" << inv << std::endl;

    inv_a.assign(y, 2);
    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 1);
    inv = inv_a & inv_b;
    std::cout << "{x = 1, y = 2} & {x <= 1}:\n\t" << inv << std::endl;

    inv_b += (z_var(z) <= 4);
    inv = inv_a & inv_b;
    std::cout << "{x = 1, y = 2} & {x <= 1, z <= 4}:\n\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();

    inv_a.assign(x, 1);
    inv_a += (z_var(y) <= 2);
    inv_a.assign(z, 3);
    inv_a += (z_var(a) >= 4);
    inv_a.assign(b, 5);

    inv_b += (z_var(y) <= 3);
    inv_b += (z_var(a) >= 1);
    inv_b.assign(z, 3);
    inv_b.set(x, interval_t(-1, 1));

    inv = inv_a & inv_b;
    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} & {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 1}:\n\t"
              << inv << std::endl;

    inv_b += (z_var(a) >= 5);
    inv = inv_a & inv_b;
    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} & {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 5}:\n\t"
              << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.assign(x, 0);
    inv_b.assign(x, 10);
    inv_a = inv_a | inv_b;
    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 5);
    inv = inv_a & inv_b;
    std::cout << "({x = 0} U {x = 10}) & {x <= 5}\n\t" << inv << std::endl;

    std::cout
        << "# Test operator||() ##########################################"
        << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::bottom();
    inv = inv_a || inv_b;
    std::cout << "top() || bottom():\n\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.set(x, interval_t(0, 1));
    inv_b.set(x, interval_t(0, 2));
    inv = inv_a || inv_b;
    std::cout << "{0 <= x <= 1} || {0 <= x <= 2}:\n\t" << inv << std::endl;

    std::cout
        << "# Test operator&&() ##########################################"
        << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::bottom();
    inv = inv_a && inv_b;
    std::cout << "top() && bottom():\n\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.set(x, interval_t(0, bound_t::plus_infinity()));
    inv_b.set(x, interval_t(0, 4));
    inv = inv_a && inv_b;
    std::cout << "{0 <= x} && {0 <= x <= 4}:\n\t" << inv << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
