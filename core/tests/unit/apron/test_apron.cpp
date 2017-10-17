#include <iostream>

#include <ikos/domains/apron.hpp>

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef variable< z_number, varname_t > z_var;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef interval< z_number > interval_t;
typedef bound< z_number > bound_t;

// typedef apron_domain< apron::interval, z_number, varname_t > domain_t;
// typedef apron_domain< apron::pkgrid_polyhedra_lin_congruences, z_number,
// varname_t > domain_t;
typedef apron_domain< apron::polka_polyhedra, z_number, varname_t > domain_t;
// typedef apron_domain< apron::ppl_linear_congruences, z_number, varname_t >
// domain_t;
// typedef apron_domain< apron::pkgrid_polyhedra_lin_congruences, z_number,
// varname_t > domain_t;

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
        << "# Test top() and bottom() ####################################"
        << std::endl;

    domain_t inv = domain_t::top();
    std::cout << "Top:\n\t" << inv << std::endl;

    inv = domain_t::bottom();
    std::cout << "Bottom:\n\t" << inv << std::endl;

    std::cout
        << "# Test assign() ##############################################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 5);
    std::cout << "Assign(x, 5):\n\t" << inv << std::endl;

    inv.assign(y, z_var(x) + 1);
    std::cout << "Assign(y, x+1):\n\t" << inv << std::endl;

    inv.assign(z, 2 * z_var(x) + 3 * z_var(y) + 1);
    std::cout << "Assign(z, 2*x + 3*y + 1):\n\t" << inv << std::endl;

    inv.assign(c, 2 * z_var(a) + 3 * z_var(b) + 1);
    std::cout << "Assign(c, 2*a + 3*b + 1):\n\t" << inv << std::endl;

    std::cout
        << "# Test operator-=() ##########################################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 2);
    inv.assign(y, 3);
    inv.assign(z, 6);
    std::cout << "Top():" << std::endl
              << "Assign(x, 2):" << std::endl
              << "Assign(y, 3):" << std::endl
              << "Assign(z, 6):" << std::endl
              << "\t" << inv << std::endl;

    inv -= x;
    std::cout << "operator-=(x):\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 2);
    inv.assign(y, 3);
    inv.assign(z, 4);
    inv.assign(a, 5);
    inv.assign(b, 6);
    inv.assign(c, 7);
    std::cout << "Top():" << std::endl
              << "Assign(x, 2):" << std::endl
              << "Assign(y, 3):" << std::endl
              << "Assign(z, 4):" << std::endl
              << "Assign(a, 5):" << std::endl
              << "Assign(b, 6):" << std::endl
              << "Assign(c, 7):" << std::endl
              << "\t" << inv << std::endl;

    patricia_tree_set< varname_t > vars;
    vars += b;
    vars += y;
    inv.forget(vars.begin(), vars.end());
    std::cout << "operator-=({b, y}):\n\t" << inv << std::endl;

    std::cout
        << "# Test apply() ###############################################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 2);
    inv.assign(y, 3);
    inv.apply(OP_MULTIPLICATION, z, x, y);
    std::cout << "Top():" << std::endl
              << "Assign(x, 2):" << std::endl
              << "Assign(y, 3):" << std::endl
              << "Apply(OP_MULTIPLICATION, z, x, y):" << std::endl
              << "\t" << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 3);
    inv.apply(OP_MULTIPLICATION, z, x, y);
    inv -= y;
    std::cout << "Top():" << std::endl
              << "Assign(x, 3):" << std::endl
              << "Apply(OP_MULTIPLICATION, z, x, y):" << std::endl
              << "operator-=(y):" << std::endl
              << "\t" << inv << std::endl;

    std::cout
        << "# Test operator+=() ##########################################"
        << std::endl;

    inv = domain_t::top();
    inv += (z_var(x) <= 2);
    std::cout << "Top():" << std::endl
              << "operator+=(x <= 2):\n\t" << inv << std::endl;

    inv += (z_var(a) == 2);
    std::cout << "operator+=(a == 2):\n\t" << inv << std::endl;

    inv += (z_var(b) != 2);
    std::cout << "operator+=(b != 2):\n\t" << inv << std::endl;

    inv -= a;
    inv -= b;
    std::cout << "operator-=(a):" << std::endl
              << "operator-=(b):" << std::endl
              << "\t" << inv << std::endl;

    inv += (2 * z_var(a) + 3 * z_var(b) >= 42);
    std::cout << "operator+=(2*a + 3*b >= 42):\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv += (z_var(a) <= 1);
    std::cout << "Top():" << std::endl
              << "operator+=(a <= 1):" << std::endl
              << "\t" << inv << std::endl;

    inv += (z_var(a) >= 3);
    std::cout << "operator+=(a >= 3):\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv.assign(x, 42);
    std::cout << "Top():" << std::endl
              << "Assign(x, 42):" << std::endl
              << "\t" << inv << std::endl;

    inv += (z_var(x) != 42);
    std::cout << "operator+=(x != 42):\n\t" << inv << std::endl;

    inv = domain_t::top();
    inv += linear_constraint_t(0, linear_constraint_t::EQUALITY);
    std::cout << "Top():" << std::endl
              << "operator+=(0 == 0):" << std::endl
              << "\t" << inv << std::endl;

    inv += linear_constraint_t(1, linear_constraint_t::EQUALITY);
    std::cout << "operator+=(1 == 0):\n\t" << inv << std::endl;

    std::cout
        << "# Test operator[]() ##########################################"
        << std::endl;

    inv = domain_t::top();
    inv.assign(x, 2);
    inv.assign(y, 3);
    inv.apply(OP_DIVISION, z, y, 2);
    std::cout << "Top():" << std::endl
              << "Assign(x, 2):" << std::endl
              << "Assign(y, 3):" << std::endl
              << "Apply(OP_DIVISION, z, y, 2):" << std::endl
              << "\t" << inv << std::endl;

    std::cout << "inv[x]: " << inv[x] << std::endl;
    std::cout << "inv[z]: " << inv[z] << std::endl;
    std::cout << "inv[a]: " << inv[a] << std::endl;

    std::cout
        << "# Test set() #################################################"
        << std::endl;

    inv.set(x, interval_t(0, 4));
    inv.set(y, interval_t(bound_t::minus_infinity(), 42));
    inv.set(z, interval_t(42, bound_t::plus_infinity()));
    std::cout << "set(x, [0, 4]):" << std::endl
              << "set(y, ]-oo, 42]):" << std::endl
              << "set(z, [42, +oo[):" << std::endl
              << "\t" << inv << std::endl;

    std::cout
        << "# Test operator<=() ##########################################"
        << std::endl;

    std::cout << "top() <= bottom():\n\t"
              << (domain_t::top() <= domain_t::bottom()) << std::endl;
    std::cout << "bottom() <= top():\n\t"
              << (domain_t::bottom() <= domain_t::top()) << std::endl;

    domain_t inv_a = domain_t::top();
    domain_t inv_b = domain_t::top();
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
        << "# Test operator|() ###########################################"
        << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::bottom();
    inv = inv_a | inv_b;
    std::cout << "top() U bottom():\n\t" << inv << std::endl;

    inv_a = domain_t::top();
    inv_b = domain_t::top();
    inv_a.assign(x, 1);
    inv = inv_a | inv_b;
    std::cout << "{x = 1} U top():\n\t" << inv << std::endl;

    inv_b += (z_var(x) <= 1);
    inv = inv_a | inv_b;
    std::cout << "{x = 1} U {x <= 1}:\n\t" << inv << std::endl;

    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 0);
    inv = inv_a | inv_b;
    std::cout << "{x = 1} U {x <= 0}:\n\t" << inv << std::endl;

    inv_a.assign(y, 2);
    inv_b = domain_t::top();
    inv_b += (z_var(x) <= 1);
    inv = inv_a | inv_b;
    std::cout << "{x = 1, y = 2} U {x <= 1}:\n\t" << inv << std::endl;

    inv_b += (z_var(z) <= 4);
    inv = inv_a | inv_b;
    std::cout << "{x = 1, y = 2} U {x <= 1, z <= 4}:\n\t" << inv << std::endl;

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

    inv = inv_a | inv_b;
    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} U {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 1}:\n\t"
              << inv << std::endl;

    inv_b += (z_var(a) >= 5);
    inv = inv_a | inv_b;
    std::cout << "{x = 1, y <= 2, z = 3, a >= 4, b = 5} U {-1 <= x <= 1, y <= "
                 "3, z = 3, a >= 5}:\n\t"
              << inv << std::endl;

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
