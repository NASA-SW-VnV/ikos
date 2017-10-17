#include <iostream>
#include <boost/optional/optional_io.hpp>

#include <ikos/domains/gauges.hpp>

/*
 * Test helpers
 */

unsigned int tests = 0;
unsigned int failed = 0;

template < typename T >
void check_print(T x) {
  std::cout << x;
}

template <>
void check_print(const bool& x) {
  std::cout << (x ? "true" : "false");
}

template < typename T >
void check_eq(const T& x, const T& y) {
  tests++;
  check_print(x);
  if (x == y) {
    std::cout << " \033[32;1m[OK]\033[0m";
  } else {
    failed++;
    std::cout << " \033[31;1m[ERROR]\033[0m";
  }
  std::cout << std::endl;
}

void end_tests() {
  if (failed) {
    std::cout << failed << " / " << tests << " tests failed!" << std::endl;
  } else {
    std::cout << tests << " / " << tests << " tests passed!" << std::endl;
  }
}

/*
 * IKOS
 */

using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef variable< z_number, varname_t > variable_t;
typedef bound< z_number > bound_t;
typedef constant< z_number > constant_t;
typedef interval< z_number > interval_t;
typedef gauge_bound< z_number, varname_t > gauge_bound_t;
typedef gauge< z_number, varname_t > gauge_t;
typedef gauge_semilattice< z_number, varname_t > gauge_semilattice_t;
typedef gauge_domain< z_number, varname_t > gauge_domain_t;
typedef linear_expression< z_number, varname_t > linear_expression_t;

void test_gauge_bound(gauge_bound_t g,
                      bool is_infinite,
                      bool is_plus_infinity,
                      bool is_minus_infinity,
                      std::size_t num_coeffs,
                      bool is_constant,
                      int constant = 0) {
  std::cout << g << std::endl;
  std::cout << "  is_infinite: ";
  check_eq(g.is_infinite(), is_infinite);
  std::cout << "  is_plus_infinity: ";
  check_eq(g.is_plus_infinity(), is_plus_infinity);
  std::cout << "  is_minus_infinity: ";
  check_eq(g.is_minus_infinity(), is_minus_infinity);
  if (g.is_finite()) {
    std::cout << "  num_coeffs: ";
    check_eq(g.num_coeffs(), num_coeffs);
  }
  std::cout << "  is_constant: ";
  check_eq(g.is_constant(), is_constant);
  if (g.is_constant()) {
    std::cout << "  constant: ";
    check_eq(g.constant(), z_number(constant));
  }
}

void test_gauge_bound_coeff(gauge_bound_t g, varname_t v, int c) {
  std::cout << "(" << g << ").operator[" << v << "] = ";
  check_eq(g[v], z_number(c));
}

void test_gauge_bound_num_bound(gauge_bound_t g, boost::optional< bound_t > b) {
  std::cout << "(" << g << ").num_bound() = ";
  check_eq(g.num_bound(), b);
}

void test_gauge_bound_add(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
  std::cout << x << " + " << y << " = ";
  check_eq(x + y, z);
}

void test_gauge_bound_sub(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
  std::cout << x << " - " << y << " = ";
  check_eq(x - y, z);
}

void test_gauge_bound_neg(gauge_bound_t x, gauge_bound_t y) {
  std::cout << "- (" << x << ") = ";
  check_eq(-x, y);
}

void test_gauge_bound_mul(gauge_bound_t x, int y, gauge_bound_t z) {
  std::cout << "(" << x << ") * " << y << " = ";
  check_eq(x * z_number(y), z);
}

void test_gauge_bound_le(gauge_bound_t x, gauge_bound_t y, bool result) {
  std::cout << x << " <= " << y << " : ";
  check_eq(x <= y, result);
}

void test_gauge_bound_ge(gauge_bound_t x, gauge_bound_t y, bool result) {
  std::cout << x << " >= " << y << " : ";
  check_eq(x >= y, result);
}

void test_gauge_bound_min(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
  std::cout << "min(" << x << ", " << y << ") = ";
  check_eq(gauge_bound_t::min(x, y), z);
}

void test_gauge_bound_max(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
  std::cout << "max(" << x << ", " << y << ") = ";
  check_eq(gauge_bound_t::max(x, y), z);
}

void test_gauge(gauge_t g,
                bool is_bottom,
                bool is_top,
                bool is_singleton,
                gauge_bound_t singleton = 0) {
  std::cout << g << std::endl;
  std::cout << "  is_bottom: ";
  check_eq(g.is_bottom(), is_bottom);
  std::cout << "  is_top: ";
  check_eq(g.is_top(), is_top);
  std::cout << "  is_singleton: ";
  check_eq(!!g.singleton(), is_singleton);
  if (g.singleton()) {
    std::cout << "  singleton: ";
    check_eq(*g.singleton(), singleton);
  }
}

void test_gauge_interval(gauge_t g, boost::optional< interval_t > i) {
  std::cout << g << ".interval() = ";
  check_eq(g.interval(), i);
}

void test_gauge_le(gauge_t x, gauge_t y, bool result) {
  std::cout << x << " <= " << y << " : ";
  check_eq(x <= y, result);
}

void test_gauge_eq(gauge_t x, gauge_t y, bool result) {
  std::cout << x << " == " << y << " : ";
  check_eq(x == y, result);
}

void test_gauge_neg(gauge_t x, gauge_t y) {
  std::cout << "- (" << x << ") = ";
  check_eq(-x, y);
}

void test_gauge_add(gauge_t x, gauge_t y, gauge_t z) {
  std::cout << x << " + " << y << " = ";
  check_eq(x + y, z);
}

void test_gauge_sub(gauge_t x, gauge_t y, gauge_t z) {
  std::cout << x << " - " << y << " = ";
  check_eq(x - y, z);
}

void test_gauge_mul(gauge_t x, int c, gauge_t y) {
  std::cout << x << " * " << c << " = ";
  check_eq(x * z_number(c), y);
}

void test_gauge_mul(gauge_t x, interval_t i, gauge_t y) {
  std::cout << x << " * " << i << " = ";
  check_eq(x * i, y);
}

void test_gauge_join(gauge_t x, gauge_t y, gauge_t z) {
  std::cout << x << " | " << y << " = ";
  check_eq(x | y, z);
}

void test_gauge_meet(gauge_t x, gauge_t y, gauge_t z) {
  std::cout << x << " & " << y << " = ";
  check_eq(x & y, z);
}

void test_gauge_forget(gauge_t x, varname_t v, gauge_t y) {
  std::cout << x << ".forget(" << v << ") = ";
  check_eq(x.forget(v), y);
}

void test_gauge_coalesce(gauge_t x, varname_t v, int l, bound_t u, gauge_t y) {
  std::cout << x << ".coalesce(" << v << ", [" << l << ", " << u << "]) = ";
  check_eq(x.coalesce(v, z_number(l), u), y);
}

void test_gauge_incr_counter(gauge_t x, varname_t v, int k, gauge_t y) {
  std::cout << x << ".incr_counter(" << v << ", " << k << ") = ";
  check_eq(x.incr_counter(v, z_number(k)), y);
}

void test_gauge_widen_interpol(
    gauge_t x, gauge_t y, varname_t k, int u, constant_t v, gauge_t z) {
  std::cout << x << " widen^{" << k << "}_{" << u << ", " << v << "} " << y
            << " = ";
  check_eq(x.widen_interpol(y, k, z_number(u), v), z);
}

void test_gauge_widen_interval(gauge_t x, gauge_t y, gauge_t z) {
  std::cout << x << " widen_{I} " << y << " = ";
  check_eq(x.widen_interval(y), z);
}

void test_gauge_semilattice(gauge_semilattice_t l,
                            bool is_bottom,
                            bool is_top) {
  std::cout << l << std::endl;
  std::cout << "  is_bottom: ";
  check_eq(l.is_bottom(), is_bottom);
  std::cout << "  is_top: ";
  check_eq(l.is_top(), is_top);
}

void test_gauge_semilattice_le(gauge_semilattice_t x,
                               gauge_semilattice_t y,
                               bool result) {
  std::cout << x << " <= " << y << " : ";
  check_eq(x <= y, result);
}

void test_gauge_semilattice_join(gauge_semilattice_t x,
                                 gauge_semilattice_t y,
                                 gauge_semilattice_t z) {
  std::cout << x << " | " << y << " = ";
  check_eq(x | y, z);
}

void test_gauge_semilattice_meet(gauge_semilattice_t x,
                                 gauge_semilattice_t y,
                                 gauge_semilattice_t z) {
  std::cout << x << " & " << y << " = ";
  check_eq(x & y, z);
}

void test_gauge_semilattice_forget(gauge_semilattice_t x,
                                   varname_t v,
                                   gauge_semilattice_t y) {
  std::cout << x << " -= " << v << " : ";
  x -= v;
  check_eq(x, y);
}

void test_gauge_semilattice_incr_counter(gauge_semilattice_t x,
                                         varname_t v,
                                         int k,
                                         gauge_semilattice_t y) {
  std::cout << x << ".incr_counter(" << v << ", " << k << ") = ";
  x.incr_counter(v, z_number(k));
  check_eq(x, y);
}

void test_gauge_semilattice_forget_counter(gauge_semilattice_t x,
                                           varname_t v,
                                           gauge_semilattice_t y) {
  std::cout << x << ".forget_counter(" << v << ") = ";
  x.forget_counter(v);
  check_eq(x, y);
}

void test_gauge_semilattice_forget_counter(gauge_semilattice_t x,
                                           varname_t v,
                                           interval_t value,
                                           gauge_semilattice_t y) {
  std::cout << x << ".forget_counter(" << v << ", " << value << ") = ";
  x.forget_counter(v, value);
  check_eq(x, y);
}

void test_gauge_semilattice_get(gauge_semilattice_t x, varname_t v, gauge_t y) {
  std::cout << x << ".operator[" << v << "] = ";
  check_eq(x[v], y);
}

void test_gauge_domain(gauge_domain_t d, bool is_bottom, bool is_top) {
  std::cout << d << std::endl;
  std::cout << "  is_bottom: ";
  check_eq(d.is_bottom(), is_bottom);
  std::cout << "  is_top: ";
  check_eq(d.is_top(), is_top);
}

void test_gauge_domain_get(gauge_domain_t x, varname_t v, gauge_t y) {
  std::cout << x << ".operator[" << v << "] = ";
  check_eq(x[v], y);
}

void test_gauge_domain_to_interval(gauge_domain_t x,
                                   varname_t v,
                                   interval_t y) {
  std::cout << x << ".to_interval(" << v << ") = ";
  check_eq(x.to_interval(v), y);
}

int main(int /*argc*/, char** /*argv*/) {
  string_factory vfac;

  try {
    varname_t x(vfac["x"]);
    varname_t y(vfac["y"]);
    varname_t z(vfac["z"]);
    varname_t w(vfac["w"]);
    varname_t i(vfac["i"]);
    varname_t k(vfac["k"]);
    varname_t zero(vfac["zero"]);
    varname_t one(vfac["one"]);
    varname_t two(vfac["two"]);

    std::cout
        << "# Test gauge bound ###########################################"
        << std::endl;

    test_gauge_bound(gauge_bound_t::plus_infinity(),
                     true,
                     true,
                     false,
                     0,
                     false);
    test_gauge_bound(gauge_bound_t::minus_infinity(),
                     true,
                     false,
                     true,
                     0,
                     false);
    test_gauge_bound(gauge_bound_t(0), false, false, false, 0, true, 0);
    test_gauge_bound(gauge_bound_t(1), false, false, false, 0, true, 1);
    test_gauge_bound(gauge_bound_t(x), false, false, false, 1, false);
    test_gauge_bound(gauge_bound_t(2, x), false, false, false, 1, false);

    gauge_bound_t g(1);
    g = g + gauge_bound_t(2, x);
    g = g + gauge_bound_t(3, y);
    test_gauge_bound(g, false, false, false, 2, false);

    std::cout
        << "# Test operator[] ############################################"
        << std::endl;

    test_gauge_bound_coeff(gauge_bound_t(0), x, 0);
    test_gauge_bound_coeff(gauge_bound_t(1), x, 0);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(x), x, 1);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(-1, x), x, -1);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(y), x, 0);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(-1, y), x, 0);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(x) +
                               gauge_bound_t(y),
                           x,
                           1);
    test_gauge_bound_coeff(gauge_bound_t(0) + gauge_bound_t(-1, x) +
                               gauge_bound_t(y),
                           x,
                           -1);

    std::cout
        << "# Test num_bound() ###########################################"
        << std::endl;

    test_gauge_bound_num_bound(gauge_bound_t::minus_infinity(),
                               boost::optional< bound_t >(
                                   bound_t::minus_infinity()));
    test_gauge_bound_num_bound(gauge_bound_t::plus_infinity(),
                               boost::optional< bound_t >(
                                   bound_t::plus_infinity()));
    test_gauge_bound_num_bound(gauge_bound_t(1),
                               boost::optional< bound_t >(bound_t(1)));
    test_gauge_bound_num_bound(gauge_bound_t(42),
                               boost::optional< bound_t >(bound_t(42)));
    test_gauge_bound_num_bound(gauge_bound_t(x), boost::optional< bound_t >());
    test_gauge_bound_num_bound(gauge_bound_t(3) + gauge_bound_t(2, x),
                               boost::optional< bound_t >());

    std::cout
        << "# Test add ###################################################"
        << std::endl;

    test_gauge_bound_add(gauge_bound_t::plus_infinity(),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_add(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_add(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_add(gauge_bound_t::minus_infinity(),
                         gauge_bound_t::minus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_add(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_add(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_add(gauge_bound_t(1),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_add(gauge_bound_t(1), gauge_bound_t(2), gauge_bound_t(3));
    test_gauge_bound_add(gauge_bound_t(1),
                         gauge_bound_t(2, x),
                         gauge_bound_t(1) + gauge_bound_t(2, x));
    test_gauge_bound_add(gauge_bound_t(x),
                         gauge_bound_t(2, x),
                         gauge_bound_t(3, x));
    test_gauge_bound_add(gauge_bound_t(x),
                         gauge_bound_t(2, y),
                         gauge_bound_t(x) + gauge_bound_t(2, y));
    test_gauge_bound_add(gauge_bound_t(x) + gauge_bound_t(2, y),
                         gauge_bound_t(3, y) + gauge_bound_t(4, z),
                         gauge_bound_t(x) + gauge_bound_t(5, y) +
                             gauge_bound_t(4, z));

    std::cout
        << "# Test sub ###################################################"
        << std::endl;

    test_gauge_bound_sub(gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_sub(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_sub(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_sub(gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_sub(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_sub(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_sub(gauge_bound_t(1),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_sub(gauge_bound_t(1), gauge_bound_t(2), gauge_bound_t(-1));
    test_gauge_bound_sub(gauge_bound_t(1),
                         gauge_bound_t(2, x),
                         gauge_bound_t(1) + gauge_bound_t(-2, x));
    test_gauge_bound_sub(gauge_bound_t(x),
                         gauge_bound_t(2, x),
                         gauge_bound_t(-1, x));
    test_gauge_bound_sub(gauge_bound_t(x),
                         gauge_bound_t(2, y),
                         gauge_bound_t(x) + gauge_bound_t(-2, y));
    test_gauge_bound_sub(gauge_bound_t(x) + gauge_bound_t(2, y),
                         gauge_bound_t(3, y) + gauge_bound_t(4, z),
                         gauge_bound_t(x) + gauge_bound_t(-1, y) +
                             gauge_bound_t(-4, z));

    std::cout
        << "# Test neg ###################################################"
        << std::endl;

    test_gauge_bound_neg(gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_neg(gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_neg(gauge_bound_t(1), gauge_bound_t(-1));
    test_gauge_bound_neg(gauge_bound_t(-2), gauge_bound_t(2));
    test_gauge_bound_neg(gauge_bound_t(2, x), gauge_bound_t(-2, x));
    test_gauge_bound_neg(gauge_bound_t(2) + gauge_bound_t(x) +
                             gauge_bound_t(3, y),
                         gauge_bound_t(-2) + gauge_bound_t(-1, x) +
                             gauge_bound_t(-3, y));

    std::cout
        << "# Test mul ###################################################"
        << std::endl;

    test_gauge_bound_mul(gauge_bound_t::plus_infinity(),
                         2,
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_mul(gauge_bound_t::plus_infinity(),
                         -2,
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_mul(gauge_bound_t::plus_infinity(), 0, gauge_bound_t(0));
    test_gauge_bound_mul(gauge_bound_t::minus_infinity(),
                         2,
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_mul(gauge_bound_t::minus_infinity(),
                         -2,
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_mul(gauge_bound_t::minus_infinity(), 0, gauge_bound_t(0));
    test_gauge_bound_mul(gauge_bound_t(1), 2, gauge_bound_t(2));
    test_gauge_bound_mul(gauge_bound_t(1), -2, gauge_bound_t(-2));
    test_gauge_bound_mul(gauge_bound_t(1), 0, gauge_bound_t(0));
    test_gauge_bound_mul(gauge_bound_t(2, x), 2, gauge_bound_t(4, x));
    test_gauge_bound_mul(gauge_bound_t(2, x), -2, gauge_bound_t(-4, x));
    test_gauge_bound_mul(gauge_bound_t(2, x), 0, gauge_bound_t(0));
    test_gauge_bound_mul(gauge_bound_t(2) + gauge_bound_t(x) +
                             gauge_bound_t(-3, y),
                         2,
                         gauge_bound_t(4) + gauge_bound_t(2, x) +
                             gauge_bound_t(-6, y));
    test_gauge_bound_mul(gauge_bound_t(2) + gauge_bound_t(x) +
                             gauge_bound_t(-3, y),
                         -2,
                         gauge_bound_t(-4) + gauge_bound_t(-2, x) +
                             gauge_bound_t(6, y));
    test_gauge_bound_mul(gauge_bound_t(2) + gauge_bound_t(x) +
                             gauge_bound_t(-3, y),
                         0,
                         gauge_bound_t(0));

    std::cout
        << "# Test <= ####################################################"
        << std::endl;

    test_gauge_bound_le(gauge_bound_t::plus_infinity(),
                        gauge_bound_t::plus_infinity(),
                        true);
    test_gauge_bound_le(gauge_bound_t::plus_infinity(),
                        gauge_bound_t::minus_infinity(),
                        false);
    test_gauge_bound_le(gauge_bound_t::plus_infinity(),
                        gauge_bound_t(1),
                        false);
    test_gauge_bound_le(gauge_bound_t::plus_infinity(),
                        gauge_bound_t(2, x),
                        false);
    test_gauge_bound_le(gauge_bound_t::minus_infinity(),
                        gauge_bound_t::plus_infinity(),
                        true);
    test_gauge_bound_le(gauge_bound_t::minus_infinity(),
                        gauge_bound_t::minus_infinity(),
                        true);
    test_gauge_bound_le(gauge_bound_t::minus_infinity(),
                        gauge_bound_t(1),
                        true);
    test_gauge_bound_le(gauge_bound_t::minus_infinity(),
                        gauge_bound_t(2, x),
                        true);
    test_gauge_bound_le(gauge_bound_t(1), gauge_bound_t::plus_infinity(), true);
    test_gauge_bound_le(gauge_bound_t(1), gauge_bound_t(2), true);
    test_gauge_bound_le(gauge_bound_t(1), gauge_bound_t(0), false);
    test_gauge_bound_le(gauge_bound_t(1), gauge_bound_t(2, x), false);
    test_gauge_bound_le(gauge_bound_t(1),
                        gauge_bound_t(2) + gauge_bound_t(2, x),
                        true);
    test_gauge_bound_le(gauge_bound_t(x), gauge_bound_t(2, x), true);
    test_gauge_bound_le(gauge_bound_t(x), gauge_bound_t(2, y), false);
    test_gauge_bound_le(gauge_bound_t(x) + gauge_bound_t(2, y),
                        gauge_bound_t(3, y) + gauge_bound_t(4, z),
                        false);

    std::cout
        << "# Test >= ####################################################"
        << std::endl;

    test_gauge_bound_ge(gauge_bound_t::plus_infinity(),
                        gauge_bound_t::plus_infinity(),
                        true);
    test_gauge_bound_ge(gauge_bound_t::plus_infinity(),
                        gauge_bound_t::minus_infinity(),
                        true);
    test_gauge_bound_ge(gauge_bound_t::plus_infinity(), gauge_bound_t(1), true);
    test_gauge_bound_ge(gauge_bound_t::plus_infinity(),
                        gauge_bound_t(2, x),
                        true);
    test_gauge_bound_ge(gauge_bound_t::minus_infinity(),
                        gauge_bound_t::plus_infinity(),
                        false);
    test_gauge_bound_ge(gauge_bound_t::minus_infinity(),
                        gauge_bound_t::minus_infinity(),
                        true);
    test_gauge_bound_ge(gauge_bound_t::minus_infinity(),
                        gauge_bound_t(1),
                        false);
    test_gauge_bound_ge(gauge_bound_t::minus_infinity(),
                        gauge_bound_t(2, x),
                        false);
    test_gauge_bound_ge(gauge_bound_t(1),
                        gauge_bound_t::plus_infinity(),
                        false);
    test_gauge_bound_ge(gauge_bound_t(1), gauge_bound_t(2), false);
    test_gauge_bound_ge(gauge_bound_t(1), gauge_bound_t(0), true);
    test_gauge_bound_ge(gauge_bound_t(1), gauge_bound_t(2, x), false);
    test_gauge_bound_ge(gauge_bound_t(2) + gauge_bound_t(2, x),
                        gauge_bound_t(1),
                        true);
    test_gauge_bound_ge(gauge_bound_t(3, x), gauge_bound_t(2, x), true);
    test_gauge_bound_ge(gauge_bound_t(x), gauge_bound_t(2, y), false);
    test_gauge_bound_ge(gauge_bound_t(x) + gauge_bound_t(2, y),
                        gauge_bound_t(3, y) + gauge_bound_t(4, z),
                        false);

    std::cout
        << "# Test min ###################################################"
        << std::endl;

    test_gauge_bound_min(gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_min(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t(1));
    test_gauge_bound_min(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t(2, x));
    test_gauge_bound_min(gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_min(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_min(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::minus_infinity());
    test_gauge_bound_min(gauge_bound_t(1),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t(1));
    test_gauge_bound_min(gauge_bound_t(1), gauge_bound_t(2), gauge_bound_t(1));
    test_gauge_bound_min(gauge_bound_t(1),
                         gauge_bound_t(2, x),
                         gauge_bound_t(0));
    test_gauge_bound_min(gauge_bound_t(x),
                         gauge_bound_t(2, x),
                         gauge_bound_t(x));
    test_gauge_bound_min(gauge_bound_t(x),
                         gauge_bound_t(2, y),
                         gauge_bound_t(0));
    test_gauge_bound_min(gauge_bound_t(x) + gauge_bound_t(2, y),
                         gauge_bound_t(3, y) + gauge_bound_t(4, z),
                         gauge_bound_t(2, y));
    test_gauge_bound_min(gauge_bound_t(4) + gauge_bound_t(x) +
                             gauge_bound_t(4, y),
                         gauge_bound_t(2) + gauge_bound_t(3, y) +
                             gauge_bound_t(-4, z),
                         gauge_bound_t(2) + gauge_bound_t(3, y) +
                             gauge_bound_t(-4, z));

    std::cout
        << "# Test max ###################################################"
        << std::endl;

    test_gauge_bound_max(gauge_bound_t::plus_infinity(),
                         gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_max(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_max(gauge_bound_t::plus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_max(gauge_bound_t::minus_infinity(),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_max(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(1),
                         gauge_bound_t(1));
    test_gauge_bound_max(gauge_bound_t::minus_infinity(),
                         gauge_bound_t(2, x),
                         gauge_bound_t(2, x));
    test_gauge_bound_max(gauge_bound_t(1),
                         gauge_bound_t::plus_infinity(),
                         gauge_bound_t::plus_infinity());
    test_gauge_bound_max(gauge_bound_t(1), gauge_bound_t(2), gauge_bound_t(2));
    test_gauge_bound_max(gauge_bound_t(1),
                         gauge_bound_t(2, x),
                         gauge_bound_t(1) + gauge_bound_t(2, x));
    test_gauge_bound_max(gauge_bound_t(x),
                         gauge_bound_t(2, x),
                         gauge_bound_t(2, x));
    test_gauge_bound_max(gauge_bound_t(x),
                         gauge_bound_t(2, y),
                         gauge_bound_t(x) + gauge_bound_t(2, y));
    test_gauge_bound_max(gauge_bound_t(x) + gauge_bound_t(2, y),
                         gauge_bound_t(3, y) + gauge_bound_t(4, z),
                         gauge_bound_t(x) + gauge_bound_t(3, y) +
                             gauge_bound_t(4, z));
    test_gauge_bound_max(gauge_bound_t(2) + gauge_bound_t(x) +
                             gauge_bound_t(4, y),
                         gauge_bound_t(4) + gauge_bound_t(3, y) +
                             gauge_bound_t(-4, z),
                         gauge_bound_t(4) + gauge_bound_t(x) +
                             gauge_bound_t(4, y));

    std::cout
        << "# Test gauge #################################################"
        << std::endl;

    test_gauge(gauge_t::top(), false, true, false);
    test_gauge(gauge_t::bottom(), true, false, false);
    test_gauge(gauge_t(gauge_bound_t::minus_infinity(), 0),
               false,
               false,
               false);
    test_gauge(gauge_t(0, gauge_bound_t::plus_infinity()), false, false, false);
    test_gauge(gauge_t(gauge_bound_t::minus_infinity(),
                       gauge_bound_t::plus_infinity()),
               false,
               true,
               false);
    test_gauge(gauge_t(0, 0), false, false, true, gauge_bound_t(0));
    test_gauge(gauge_t(gauge_bound_t(x)), false, false, true, gauge_bound_t(x));
    test_gauge(gauge_t(0, gauge_bound_t(x)), false, false, false);

    std::cout
        << "# Test interval() ############################################"
        << std::endl;

    test_gauge_interval(gauge_t::top(),
                        boost::optional< interval_t >(interval_t::top()));
    test_gauge_interval(gauge_t::bottom(),
                        boost::optional< interval_t >(interval_t::bottom()));
    test_gauge_interval(gauge_t(1, 2),
                        boost::optional< interval_t >(interval_t(1, 2)));
    test_gauge_interval(gauge_t(gauge_bound_t::minus_infinity(), 2),
                        boost::optional< interval_t >(
                            interval_t(bound_t::minus_infinity(), 2)));
    test_gauge_interval(gauge_t(1, gauge_bound_t::plus_infinity()),
                        boost::optional< interval_t >(
                            interval_t(1, bound_t::plus_infinity())));
    test_gauge_interval(gauge_t(0, gauge_bound_t(2, x)),
                        boost::optional< interval_t >());
    test_gauge_interval(gauge_t(gauge_bound_t(-2, x), gauge_bound_t(2, x)),
                        boost::optional< interval_t >());

    std::cout
        << "# Test <= ####################################################"
        << std::endl;

    test_gauge_le(gauge_t::top(), gauge_t::bottom(), false);
    test_gauge_le(gauge_t::top(), gauge_t::top(), true);
    test_gauge_le(gauge_t::top(), gauge_t(0, 1), false);
    test_gauge_le(gauge_t::top(), gauge_t(0, gauge_bound_t(2, x)), false);
    test_gauge_le(gauge_t::top(),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  false);
    test_gauge_le(gauge_t::top(),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  false);
    test_gauge_le(gauge_t::bottom(), gauge_t::bottom(), true);
    test_gauge_le(gauge_t::bottom(), gauge_t::top(), true);
    test_gauge_le(gauge_t::bottom(), gauge_t(0, 1), true);
    test_gauge_le(gauge_t::bottom(), gauge_t(0, gauge_bound_t(2, x)), true);
    test_gauge_le(gauge_t::bottom(),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  true);
    test_gauge_le(gauge_t::bottom(),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  true);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t::bottom(),
                  false);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t::top(),
                  true);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(0, 1),
                  false);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  true);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  false);
    test_gauge_le(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(gauge_bound_t::minus_infinity(), gauge_bound_t(2, x)),
                  false);
    test_gauge_le(gauge_t(0, 1), gauge_t(-1, 2), true);
    test_gauge_le(gauge_t(0, 1),
                  gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                  true);
    test_gauge_le(gauge_t(-2, 1), gauge_t(-1, 2), false);
    test_gauge_le(gauge_t(0, 3), gauge_t(-1, 2), false);
    test_gauge_le(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                          gauge_bound_t(3) + gauge_bound_t(3, x)),
                  gauge_t(-1,
                          gauge_bound_t(4) + gauge_bound_t(4, x) +
                              gauge_bound_t(y)),
                  true);

    std::cout
        << "# Test == ####################################################"
        << std::endl;

    test_gauge_eq(gauge_t::top(), gauge_t::bottom(), false);
    test_gauge_eq(gauge_t::top(), gauge_t::top(), true);
    test_gauge_eq(gauge_t::top(), gauge_t(0, 1), false);
    test_gauge_eq(gauge_t::top(), gauge_t(0, gauge_bound_t(2, x)), false);
    test_gauge_eq(gauge_t::top(),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  false);
    test_gauge_eq(gauge_t::top(),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  false);
    test_gauge_eq(gauge_t::bottom(), gauge_t::bottom(), true);
    test_gauge_eq(gauge_t::bottom(), gauge_t::top(), false);
    test_gauge_eq(gauge_t::bottom(), gauge_t(0, 1), false);
    test_gauge_eq(gauge_t::bottom(), gauge_t(0, gauge_bound_t(2, x)), false);
    test_gauge_eq(gauge_t::bottom(),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  false);
    test_gauge_eq(gauge_t::bottom(),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  false);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t::bottom(),
                  false);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t::top(),
                  false);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(0, 1),
                  false);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(0, gauge_bound_t::plus_infinity()),
                  true);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(gauge_bound_t::minus_infinity(), 0),
                  false);
    test_gauge_eq(gauge_t(0, gauge_bound_t::plus_infinity()),
                  gauge_t(gauge_bound_t::minus_infinity(), gauge_bound_t(2, x)),
                  false);
    test_gauge_eq(gauge_t(0, 1), gauge_t(-1, 2), false);
    test_gauge_eq(gauge_t(0, 1),
                  gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                  false);
    test_gauge_eq(gauge_t(-2, 1), gauge_t(-1, 2), false);
    test_gauge_eq(gauge_t(0, 3), gauge_t(-1, 2), false);
    test_gauge_eq(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                          gauge_bound_t(3) + gauge_bound_t(3, x)),
                  gauge_t(-1,
                          gauge_bound_t(4) + gauge_bound_t(4, x) +
                              gauge_bound_t(y)),
                  false);

    std::cout << "# Test join #################################################"
              << std::endl;

    test_gauge_join(gauge_t::top(), gauge_t::bottom(), gauge_t::top());
    test_gauge_join(gauge_t::top(), gauge_t::top(), gauge_t::top());
    test_gauge_join(gauge_t::top(), gauge_t(0, 1), gauge_t::top());
    test_gauge_join(gauge_t::top(),
                    gauge_t(0, gauge_bound_t(2, x)),
                    gauge_t::top());
    test_gauge_join(gauge_t::top(),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::top());
    test_gauge_join(gauge_t::top(),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t::top());
    test_gauge_join(gauge_t::bottom(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_join(gauge_t::bottom(), gauge_t::top(), gauge_t::top());
    test_gauge_join(gauge_t::bottom(), gauge_t(0, 1), gauge_t(0, 1));
    test_gauge_join(gauge_t::bottom(),
                    gauge_t(0, gauge_bound_t(2, x)),
                    gauge_t(0, gauge_bound_t(2, x)));
    test_gauge_join(gauge_t::bottom(),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_join(gauge_t::bottom(),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t(gauge_bound_t::minus_infinity(), 0));
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::bottom(),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::top(),
                    gauge_t::top());
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, 1),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t::top());
    test_gauge_join(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t::minus_infinity(),
                            gauge_bound_t(2, x)),
                    gauge_t::top());
    test_gauge_join(gauge_t(0, 1), gauge_t(-1, 2), gauge_t(-1, 2));
    test_gauge_join(gauge_t(0, 1),
                    gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                    gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)));
    test_gauge_join(gauge_t(-2, 1), gauge_t(-1, 2), gauge_t(-2, 2));
    test_gauge_join(gauge_t(0, 3), gauge_t(-1, 2), gauge_t(-1, 3));
    test_gauge_join(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                            gauge_bound_t(3) + gauge_bound_t(3, x)),
                    gauge_t(-1,
                            gauge_bound_t(4) + gauge_bound_t(4, x) +
                                gauge_bound_t(y)),
                    gauge_t(-1,
                            gauge_bound_t(4) + gauge_bound_t(4, x) +
                                gauge_bound_t(y)));

    std::cout
        << "# Test meet ##################################################"
        << std::endl;

    test_gauge_meet(gauge_t::top(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_meet(gauge_t::top(), gauge_t::top(), gauge_t::top());
    test_gauge_meet(gauge_t::top(), gauge_t(0, 1), gauge_t(0, 1));
    test_gauge_meet(gauge_t::top(),
                    gauge_t(0, gauge_bound_t(2, x)),
                    gauge_t(0, gauge_bound_t(2, x)));
    test_gauge_meet(gauge_t::top(),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_meet(gauge_t::top(),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t(gauge_bound_t::minus_infinity(), 0));
    test_gauge_meet(gauge_t::bottom(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_meet(gauge_t::bottom(), gauge_t::top(), gauge_t::bottom());
    test_gauge_meet(gauge_t::bottom(), gauge_t(0, 1), gauge_t::bottom());
    test_gauge_meet(gauge_t::bottom(),
                    gauge_t(0, gauge_bound_t(2, x)),
                    gauge_t::bottom());
    test_gauge_meet(gauge_t::bottom(),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::bottom());
    test_gauge_meet(gauge_t::bottom(),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t::bottom());
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::bottom(),
                    gauge_t::bottom());
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t::top(),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, 1),
                    gauge_t(0, 1));
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t::minus_infinity(), 0),
                    gauge_t(0, 0));
    test_gauge_meet(gauge_t(0, gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t::minus_infinity(),
                            gauge_bound_t(2, x)),
                    gauge_t(0, gauge_bound_t(2, x)));
    test_gauge_meet(gauge_t(0, 1), gauge_t(-1, 2), gauge_t(0, 1));
    test_gauge_meet(gauge_t(0, 1),
                    gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                    gauge_t(gauge_bound_t(0), gauge_bound_t(1)));
    test_gauge_meet(gauge_t(-2, 1), gauge_t(-1, 2), gauge_t(-1, 1));
    test_gauge_meet(gauge_t(0, 3), gauge_t(-1, 2), gauge_t(0, 2));
    test_gauge_meet(gauge_t(0, 3), gauge_t(4, 5), gauge_t::bottom());
    test_gauge_meet(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                            gauge_bound_t(3) + gauge_bound_t(3, x)),
                    gauge_t(-1,
                            gauge_bound_t(4) + gauge_bound_t(4, x) +
                                gauge_bound_t(y)),
                    gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                            gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_meet(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                            gauge_bound_t(3) + gauge_bound_t(3, x)),
                    gauge_t(gauge_bound_t(-1) + gauge_bound_t(3, y),
                            gauge_bound_t(3) + gauge_bound_t(4, y)),
                    gauge_t(gauge_bound_t(-1) + gauge_bound_t(3, y),
                            gauge_bound_t(3) + gauge_bound_t(3, x) +
                                gauge_bound_t(3, y)));
    test_gauge_meet(gauge_t(0, gauge_bound_t(y)),
                    gauge_t(gauge_bound_t(x), gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t(x),
                            gauge_bound_t(x) + gauge_bound_t(y)));
    test_gauge_meet(gauge_t(gauge_bound_t(x),
                            gauge_bound_t(1) + gauge_bound_t(x)),
                    gauge_t(gauge_bound_t(x) + gauge_bound_t(y) +
                                gauge_bound_t(1),
                            gauge_bound_t::plus_infinity()),
                    gauge_t(gauge_bound_t(x) + gauge_bound_t(y) +
                            gauge_bound_t(1)));

    std::cout
        << "# Test neg ###################################################"
        << std::endl;

    test_gauge_neg(gauge_t::top(), gauge_t::top());
    test_gauge_neg(gauge_t::bottom(), gauge_t::bottom());
    test_gauge_neg(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(), 0));
    test_gauge_neg(gauge_t(0, 1), gauge_t(-1, 0));
    test_gauge_neg(gauge_t(-2, 1), gauge_t(-1, 2));
    test_gauge_neg(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-3, x),
                           gauge_bound_t(1) + gauge_bound_t(-2, x)));

    std::cout
        << "# Test add ###################################################"
        << std::endl;

    test_gauge_add(gauge_t::top(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_add(gauge_t::top(), gauge_t::top(), gauge_t::top());
    test_gauge_add(gauge_t::top(), gauge_t(0, 1), gauge_t::top());
    test_gauge_add(gauge_t::top(),
                   gauge_t(0, gauge_bound_t(2, x)),
                   gauge_t::top());
    test_gauge_add(gauge_t::top(),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_add(gauge_t::top(),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t::top());
    test_gauge_add(gauge_t::bottom(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_add(gauge_t::bottom(), gauge_t::top(), gauge_t::bottom());
    test_gauge_add(gauge_t::bottom(), gauge_t(0, 1), gauge_t::bottom());
    test_gauge_add(gauge_t::bottom(),
                   gauge_t(0, gauge_bound_t(2, x)),
                   gauge_t::bottom());
    test_gauge_add(gauge_t::bottom(),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::bottom());
    test_gauge_add(gauge_t::bottom(),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t::bottom());
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::bottom(),
                   gauge_t::bottom());
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::top(),
                   gauge_t::top());
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(0, gauge_bound_t(1)),
                   gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t::top());
    test_gauge_add(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(2, x)),
                   gauge_t::top());
    test_gauge_add(gauge_t(0, 1), gauge_t(-1, 2), gauge_t(-1, 3));
    test_gauge_add(gauge_t(0, 1),
                   gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                   gauge_t(-1, gauge_bound_t(3) + gauge_bound_t(2, x)));
    test_gauge_add(gauge_t(-2, 1), gauge_t(-1, 2), gauge_t(-3, 3));
    test_gauge_add(gauge_t(0, 3), gauge_t(-1, 2), gauge_t(-1, 5));
    test_gauge_add(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   gauge_t(-1,
                           gauge_bound_t(4) + gauge_bound_t(4, x) +
                               gauge_bound_t(y)),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(2, x),
                           gauge_bound_t(7) + gauge_bound_t(7, x) +
                               gauge_bound_t(y)));

    std::cout
        << "# Test sub ###################################################"
        << std::endl;

    test_gauge_sub(gauge_t::top(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_sub(gauge_t::top(), gauge_t::top(), gauge_t::top());
    test_gauge_sub(gauge_t::top(), gauge_t(0, 1), gauge_t::top());
    test_gauge_sub(gauge_t::top(),
                   gauge_t(0, gauge_bound_t(2, x)),
                   gauge_t::top());
    test_gauge_sub(gauge_t::top(),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_sub(gauge_t::top(),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t::top());
    test_gauge_sub(gauge_t::bottom(), gauge_t::bottom(), gauge_t::bottom());
    test_gauge_sub(gauge_t::bottom(), gauge_t::top(), gauge_t::bottom());
    test_gauge_sub(gauge_t::bottom(), gauge_t(0, 1), gauge_t::bottom());
    test_gauge_sub(gauge_t::bottom(),
                   gauge_t(0, gauge_bound_t(2, x)),
                   gauge_t::bottom());
    test_gauge_sub(gauge_t::bottom(),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::bottom());
    test_gauge_sub(gauge_t::bottom(),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t::bottom());
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::bottom(),
                   gauge_t::bottom());
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::top(),
                   gauge_t::top());
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(0, 1),
                   gauge_t(-1, gauge_bound_t::plus_infinity()));
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(), 0),
                   gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_sub(gauge_t(0, gauge_bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(2, x)),
                   gauge_t(gauge_bound_t(-2, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_sub(gauge_t(0, 1), gauge_t(-1, 2), gauge_t(-2, 2));
    test_gauge_sub(gauge_t(0, 1),
                   gauge_t(-1, gauge_bound_t(2) + gauge_bound_t(2, x)),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(-2, x), 2));
    test_gauge_sub(gauge_t(-2, 1), gauge_t(-1, 2), gauge_t(-4, 2));
    test_gauge_sub(gauge_t(0, 3), gauge_t(-1, 2), gauge_t(-2, 4));
    test_gauge_sub(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   gauge_t(-1,
                           gauge_bound_t(4) + gauge_bound_t(4, x) +
                               gauge_bound_t(y)),
                   gauge_t(gauge_bound_t(-5) + gauge_bound_t(-2, x) +
                               gauge_bound_t(-1, y),
                           gauge_bound_t(4) + gauge_bound_t(3, x)));

    std::cout
        << "# Test operator*(c) ##########################################"
        << std::endl;

    test_gauge_mul(gauge_t::top(), 0, gauge_t(0, 0));
    test_gauge_mul(gauge_t::top(), 1, gauge_t::top());
    test_gauge_mul(gauge_t::top(), -1, gauge_t::top());
    test_gauge_mul(gauge_t::bottom(), 0, gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(), 1, gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(), -1, gauge_t::bottom());
    test_gauge_mul(gauge_t(0, gauge_bound_t::plus_infinity()),
                   0,
                   gauge_t(0, 0));
    test_gauge_mul(gauge_t(0, gauge_bound_t::plus_infinity()),
                   2,
                   gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(0, gauge_bound_t::plus_infinity()),
                   -2,
                   gauge_t(gauge_bound_t::minus_infinity(), 0));
    test_gauge_mul(gauge_t(0, 1), 0, gauge_t(0, 0));
    test_gauge_mul(gauge_t(0, 1), 2, gauge_t(0, 2));
    test_gauge_mul(gauge_t(0, 1), -2, gauge_t(-2, 0));
    test_gauge_mul(gauge_t(-2, 1), 0, gauge_t(0, 0));
    test_gauge_mul(gauge_t(-2, 1), 2, gauge_t(-4, 2));
    test_gauge_mul(gauge_t(-2, 1), -2, gauge_t(-2, 4));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   0,
                   gauge_t(0, 0));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   2,
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(4, x),
                           gauge_bound_t(6) + gauge_bound_t(6, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)),
                   -2,
                   gauge_t(gauge_bound_t(-6) + gauge_bound_t(-6, x),
                           gauge_bound_t(2) + gauge_bound_t(-4, x)));

    std::cout
        << "# Test operator*([a, b]) #####################################"
        << std::endl;

    test_gauge_mul(gauge_t::top(), interval_t::top(), gauge_t::top());
    test_gauge_mul(gauge_t::top(), interval_t::bottom(), gauge_t::bottom());
    test_gauge_mul(gauge_t::top(), interval_t(0, 1), gauge_t::top());
    test_gauge_mul(gauge_t::top(),
                   interval_t(bound_t::minus_infinity(), 1),
                   gauge_t::top());
    test_gauge_mul(gauge_t::top(),
                   interval_t(0, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t::bottom(), interval_t::top(), gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(), interval_t::bottom(), gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(), interval_t(0, 1), gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(),
                   interval_t(bound_t::minus_infinity(), 1),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t::bottom(),
                   interval_t(0, bound_t::plus_infinity()),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(1, 2), interval_t::top(), gauge_t::top());
    test_gauge_mul(gauge_t(1, 2), interval_t::bottom(), gauge_t::bottom());
    test_gauge_mul(gauge_t(1, 2),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t(gauge_bound_t::minus_infinity(), 6));
    test_gauge_mul(gauge_t(1, 2),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(gauge_bound_t::minus_infinity(), -3));
    test_gauge_mul(gauge_t(1, 2),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, 2),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t(-4, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, 2), interval_t(2, 3), gauge_t(2, 6));
    test_gauge_mul(gauge_t(1, 2), interval_t(-3, -2), gauge_t(-6, -2));
    test_gauge_mul(gauge_t(1, 2), interval_t(-2, 3), gauge_t(-4, 6));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(gauge_bound_t::minus_infinity(), -3));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(-2, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(2, 3),
                   gauge_t(2, gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-3, x), -2));
    test_gauge_mul(gauge_t(1, gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-2, 3),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(-2, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(gauge_bound_t::minus_infinity(), -3));
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(2, 3),
                   gauge_t(2, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t::minus_infinity(), -2));
    test_gauge_mul(gauge_t(1, gauge_bound_t::plus_infinity()),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(2, 3),
                   gauge_t(-3, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t::minus_infinity(), 3));
    test_gauge_mul(gauge_t(-1, gauge_bound_t::plus_infinity()),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(-3) + gauge_bound_t(-6, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t(2) + gauge_bound_t(4, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(-2) + gauge_bound_t(-4, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, x),
                           gauge_bound_t::plus_infinity()),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(3, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(), -1));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t::minus_infinity(), -2));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(-3, -2),
                   gauge_t(2, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), -1),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t::minus_infinity(), 3));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(-3, -2),
                   gauge_t(-3, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(), 1),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(3) + gauge_bound_t(9, x)));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-9, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(1) + gauge_bound_t(3, x)),
                   interval_t(-2, 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-6, x),
                           gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t(3, gauge_bound_t::plus_infinity()));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(), -1));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t(gauge_bound_t::minus_infinity(),
                           gauge_bound_t(2) + gauge_bound_t(4, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-6, x), -2));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(-3, -2),
                   gauge_t(2, gauge_bound_t(3) + gauge_bound_t(6, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(-1) + gauge_bound_t(-2, x), -1),
                   interval_t(-2, 3),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-6, x),
                           gauge_bound_t(2) + gauge_bound_t(4, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t(2) + gauge_bound_t(-3, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-3, x),
                           gauge_bound_t(-2) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x)),
                   interval_t(-2, 3),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(-3, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t::top(),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t::bottom(),
                   gauge_t::bottom());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(bound_t::minus_infinity(), 3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(bound_t::minus_infinity(), -3),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(1, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(-2, bound_t::plus_infinity()),
                   gauge_t::top());
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(2, 3),
                   gauge_t(gauge_bound_t(2) + gauge_bound_t(-3, x),
                           gauge_bound_t(3) + gauge_bound_t(3, x) +
                               gauge_bound_t(6, y)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(-3, -2),
                   gauge_t(gauge_bound_t(-3) + gauge_bound_t(-3, x) +
                               gauge_bound_t(-6, y),
                           gauge_bound_t(-2) + gauge_bound_t(3, x)));
    test_gauge_mul(gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, x),
                           gauge_bound_t(1) + gauge_bound_t(x) +
                               gauge_bound_t(2, y)),
                   interval_t(-2, 3),
                   gauge_t(gauge_bound_t(-2) + gauge_bound_t(-3, x) +
                               gauge_bound_t(-4, y),
                           gauge_bound_t(3) + gauge_bound_t(3, x) +
                               gauge_bound_t(6, y)));

    std::cout
        << "# Test forget ################################################"
        << std::endl;

    test_gauge_forget(gauge_t::top(), x, gauge_t::top());
    test_gauge_forget(gauge_t::bottom(), x, gauge_t::bottom());
    test_gauge_forget(gauge_t(0, 1), x, gauge_t(0, 1));
    test_gauge_forget(gauge_t(0, gauge_bound_t::plus_infinity()),
                      x,
                      gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_forget(gauge_t(gauge_bound_t::minus_infinity(), 1),
                      x,
                      gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_forget(gauge_t(0, gauge_bound_t(2, x)), x, gauge_t::top());
    test_gauge_forget(gauge_t(gauge_bound_t(-1, x), 2), x, gauge_t::top());
    test_gauge_forget(gauge_t(0, gauge_bound_t(2, y)),
                      x,
                      gauge_t(0, gauge_bound_t(2, y)));

    std::cout
        << "# Test coalesce ##############################################"
        << std::endl;

    test_gauge_coalesce(gauge_t::top(),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t::top());
    test_gauge_coalesce(gauge_t::bottom(),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t::bottom());
    test_gauge_coalesce(gauge_t(0, 1),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(0, 1));
    test_gauge_coalesce(gauge_t(0, gauge_bound_t::plus_infinity()),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_coalesce(gauge_t(gauge_bound_t::minus_infinity(), 1),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_coalesce(gauge_t(0, gauge_bound_t(2, x)),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_coalesce(gauge_t(gauge_bound_t(-1, x), 2),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(gauge_bound_t::minus_infinity(), 2));
    test_gauge_coalesce(gauge_t(0, gauge_bound_t(2, y)),
                        x,
                        0,
                        bound_t::plus_infinity(),
                        gauge_t(0, gauge_bound_t(2, y)));
    test_gauge_coalesce(gauge_t(gauge_bound_t(x),
                                gauge_bound_t(1) + gauge_bound_t(x)),
                        x,
                        1,
                        bound_t(2),
                        gauge_t(1, 3));
    test_gauge_coalesce(gauge_t(gauge_bound_t(2) + gauge_bound_t(-2, x),
                                gauge_bound_t(3) + gauge_bound_t(-2, x)),
                        x,
                        1,
                        bound_t(3),
                        gauge_t(-4, 1));
    test_gauge_coalesce(gauge_t(gauge_bound_t(2) + gauge_bound_t(-2, x) +
                                    gauge_bound_t(3, y),
                                gauge_bound_t(3) + gauge_bound_t(-2, x) +
                                    gauge_bound_t(3, y)),
                        x,
                        1,
                        bound_t(3),
                        gauge_t(gauge_bound_t(-4) + gauge_bound_t(3, y),
                                gauge_bound_t(1) + gauge_bound_t(3, y)));
    test_gauge_coalesce(gauge_t(gauge_bound_t(2) + gauge_bound_t(-2, x),
                                gauge_bound_t(3) + gauge_bound_t(2, x)),
                        x,
                        1,
                        bound_t::plus_infinity(),
                        gauge_t::top());
    test_gauge_coalesce(gauge_t(gauge_bound_t(2) + gauge_bound_t(-2, x) +
                                    gauge_bound_t(3, y),
                                gauge_bound_t(3) + gauge_bound_t(2, x) +
                                    gauge_bound_t(3, y)),
                        x,
                        1,
                        bound_t::plus_infinity(),
                        gauge_t::top());

    std::cout
        << "# Test incr_counter ##########################################"
        << std::endl;

    test_gauge_incr_counter(gauge_t::top(), i, 1, gauge_t::top());
    test_gauge_incr_counter(gauge_t::bottom(), i, 1, gauge_t::bottom());
    test_gauge_incr_counter(gauge_t(0, 1), i, 1, gauge_t(0, 1));
    test_gauge_incr_counter(gauge_t(0, gauge_bound_t::plus_infinity()),
                            i,
                            1,
                            gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_incr_counter(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i),
                                    gauge_bound_t::plus_infinity()),
                            i,
                            1,
                            gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, i),
                                    gauge_bound_t::plus_infinity()));
    test_gauge_incr_counter(gauge_t(gauge_bound_t::minus_infinity(), 1),
                            i,
                            1,
                            gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_incr_counter(gauge_t(gauge_bound_t::minus_infinity(),
                                    gauge_bound_t(1) + gauge_bound_t(2, i)),
                            i,
                            1,
                            gauge_t(gauge_bound_t::minus_infinity(),
                                    gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_incr_counter(gauge_t(gauge_bound_t(i)),
                            i,
                            1,
                            gauge_t(gauge_bound_t(-1) + gauge_bound_t(i),
                                    gauge_bound_t(-1) + gauge_bound_t(i)));
    test_gauge_incr_counter(gauge_t(0, gauge_bound_t(2, i)),
                            i,
                            1,
                            gauge_t(-2, gauge_bound_t(2, i)));
    test_gauge_incr_counter(gauge_t(1, gauge_bound_t(2) + gauge_bound_t(3, i)),
                            i,
                            1,
                            gauge_t(-1,
                                    gauge_bound_t(1) + gauge_bound_t(3, i)));
    test_gauge_incr_counter(gauge_t(gauge_bound_t(-2) + gauge_bound_t(2, i) +
                                        gauge_bound_t(2, k),
                                    gauge_bound_t(2) + gauge_bound_t(3, i) +
                                        gauge_bound_t(3, k)),
                            i,
                            1,
                            gauge_t(gauge_bound_t(-4) + gauge_bound_t(2, i) +
                                        gauge_bound_t(2, k),
                                    gauge_bound_t(-1) + gauge_bound_t(3, i) +
                                        gauge_bound_t(3, k)));
    test_gauge_incr_counter(gauge_t(gauge_bound_t(-2) + gauge_bound_t(2, i) +
                                        gauge_bound_t(2, k),
                                    gauge_bound_t(2) + gauge_bound_t(3, i) +
                                        gauge_bound_t(3, k)),
                            i,
                            2,
                            gauge_t(gauge_bound_t(-6) + gauge_bound_t(2, i) +
                                        gauge_bound_t(2, k),
                                    gauge_bound_t(-4) + gauge_bound_t(3, i) +
                                        gauge_bound_t(3, k)));

    std::cout
        << "# Test widening interpolation ################################"
        << std::endl;

    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t::bottom(),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t::top(),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t(0, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t(0, gauge_bound_t(2, i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t::bottom(),
                              gauge_t::bottom(),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::bottom());
    test_gauge_widen_interpol(gauge_t::bottom(),
                              gauge_t::top(),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t::bottom(),
                              gauge_t(0, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, 1));
    test_gauge_widen_interpol(gauge_t::bottom(),
                              gauge_t(0, gauge_bound_t(2, i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 1),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(0, gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 2),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(0, gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 3),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(0, gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(-2, 2),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(-2, i),
                                      gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(4, 4),
                              i,
                              0,
                              constant_t(3),
                              gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(1, 1),
                              gauge_t(2, 2),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(1, 1),
                              gauge_t(-2, -2),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(-3, i)));
    test_gauge_widen_interpol(gauge_t(gauge_bound_t(4, i)),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(4, i)),
                              k,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(4, i) + gauge_bound_t(k)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(1, 1),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(-2) + gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(3, 3),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(-6) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(-3, -3),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(6) + gauge_bound_t(-3, i)));
    test_gauge_widen_interpol(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i)),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i)),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i)),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(2, i)),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(-1) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(1,
                                      gauge_bound_t(1) + gauge_bound_t(2, i)),
                              gauge_t(2,
                                      gauge_bound_t(2) + gauge_bound_t(2, i)),
                              i,
                              2,
                              constant_t(3),
                              gauge_t(gauge_bound_t(-1) + gauge_bound_t(i),
                                      gauge_bound_t(-1) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(2, gauge_bound_t::plus_infinity()),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(2, i),
                                      gauge_bound_t::plus_infinity()));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(gauge_bound_t::minus_infinity(), 2),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t::minus_infinity(),
                                      gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t::top(),
                              i,
                              0,
                              constant_t(1),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t(gauge_bound_t::minus_infinity(), 0),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t::minus_infinity(),
                                      gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(0, gauge_bound_t::plus_infinity()),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t(1),
                              gauge_t(gauge_bound_t(i),
                                      gauge_bound_t::plus_infinity()));
    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t(1),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, 1));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, 1));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 2),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, 2));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(-2, 2),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(-2, 2));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(0, 2),
                              i,
                              1,
                              constant_t::top(),
                              gauge_t(0, 2));
    test_gauge_widen_interpol(gauge_t(gauge_bound_t(4, k)),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(4, k)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(4, k),
                                      gauge_bound_t(2) + gauge_bound_t(4, k)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(gauge_bound_t(i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(1, 1),
                              gauge_t(gauge_bound_t(i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(i),
                                      gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)),
                              i,
                              2,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(2, i)));
    test_gauge_widen_interpol(gauge_t(0, 1),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(0, 1),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)),
                              i,
                              2,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(4) + gauge_bound_t(3, i)),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(4) + gauge_bound_t(3, i)));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(2, gauge_bound_t::plus_infinity()),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t(gauge_bound_t::minus_infinity(), 2),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t::minus_infinity(), 2));
    test_gauge_widen_interpol(gauge_t(0, 0),
                              gauge_t::top(),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());
    test_gauge_widen_interpol(gauge_t(gauge_bound_t::minus_infinity(), 0),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_widen_interpol(gauge_t(0, gauge_bound_t::plus_infinity()),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interpol(gauge_t::top(),
                              gauge_t(1, 1),
                              i,
                              0,
                              constant_t::top(),
                              gauge_t::top());

    std::cout
        << "# Test widening interval #####################################"
        << std::endl;

    test_gauge_widen_interval(gauge_t::top(),
                              gauge_t::bottom(),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t::top(), gauge_t::top(), gauge_t::top());
    test_gauge_widen_interval(gauge_t::top(), gauge_t(0, 1), gauge_t::top());
    test_gauge_widen_interval(gauge_t::top(),
                              gauge_t(0, gauge_bound_t(2, i)),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t::bottom(),
                              gauge_t::bottom(),
                              gauge_t::bottom());
    test_gauge_widen_interval(gauge_t::bottom(),
                              gauge_t::top(),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t::bottom(), gauge_t(0, 1), gauge_t(0, 1));
    test_gauge_widen_interval(gauge_t::bottom(),
                              gauge_t(0, gauge_bound_t(2, i)),
                              gauge_t(0, gauge_bound_t(2, i)));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(1, 1),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(0, 1),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(0, 2),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(0, 3),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(-2, 2),
                              gauge_t(gauge_bound_t::minus_infinity(),
                                      gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 4), gauge_t(4, 4), gauge_t(0, 4));
    test_gauge_widen_interval(gauge_t(1, 1),
                              gauge_t(2, 2),
                              gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(1, 1),
                              gauge_t(-2, -2),
                              gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_widen_interval(gauge_t(gauge_bound_t(4, i)),
                              gauge_t(gauge_bound_t(1) + gauge_bound_t(4, i)),
                              gauge_t(gauge_bound_t(4, i),
                                      gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(1,
                                      gauge_bound_t(1) + gauge_bound_t(2, i)),
                              gauge_t(2,
                                      gauge_bound_t(2) + gauge_bound_t(2, i)),
                              gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(gauge_bound_t(i)),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(1, 1),
                              gauge_t(gauge_bound_t(i)),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 1),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(4) + gauge_bound_t(3, i)),
                              gauge_t(gauge_bound_t(2) + gauge_bound_t(i),
                                      gauge_bound_t(3) + gauge_bound_t(3, i)),
                              gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                                      gauge_bound_t(4) + gauge_bound_t(3, i)));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(2, gauge_bound_t::plus_infinity()),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t(0, 0),
                              gauge_t(gauge_bound_t::minus_infinity(), 2),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t(0, 0), gauge_t::top(), gauge_t::top());
    test_gauge_widen_interval(gauge_t(gauge_bound_t::minus_infinity(), 0),
                              gauge_t(1, 1),
                              gauge_t::top());
    test_gauge_widen_interval(gauge_t(0, gauge_bound_t::plus_infinity()),
                              gauge_t(1, 1),
                              gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_widen_interval(gauge_t::top(), gauge_t(1, 1), gauge_t::top());

    std::cout
        << "# Test gauge semilattice #####################################"
        << std::endl;

    gauge_semilattice_t l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13;

    l1.set(x, gauge_t(0, 1));

    l2.set(x, gauge_t(0, 1));
    l2.set(y, gauge_t(gauge_bound_t(i), gauge_bound_t(2, i)));

    l3.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));

    l4.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));
    l4.set(y,
           gauge_t(gauge_bound_t(-2) + gauge_bound_t(i), gauge_bound_t(3, i)));

    l5.set(x, gauge_t(0, 1));
    l5.set(y, gauge_t(gauge_bound_t(2, k), gauge_bound_t(2, k)));

    l6.set(x, gauge_t(0, 1));
    l6.set(y, gauge_t(0, gauge_bound_t(2, i) + gauge_bound_t(2, k)));

    l7.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));
    l7.set(y, gauge_t(-2, gauge_bound_t(3, i) + gauge_bound_t(2, k)));

    l8.set(x, gauge_t(0, 1));
    l8.set(y,
           gauge_t(gauge_bound_t(-2) + gauge_bound_t(i), gauge_bound_t(3, i)));

    l9.set(x, gauge_t(0, 1));
    l9.set(y,
           gauge_t(gauge_bound_t(2, k),
                   gauge_bound_t(2, i) + gauge_bound_t(2, k)));

    l10.set(x, gauge_t(0, 1));
    l10.set(y, gauge_t(0, gauge_bound_t::plus_infinity()));

    l11.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));
    l11.set(y, gauge_t(-2, gauge_bound_t::plus_infinity()));

    l12.set(x, gauge_t(0, 1));
    l12.set(y, gauge_t(1, 6));

    l13.set(x, gauge_t(0, 1));
    l13.set(y, gauge_t(0, gauge_bound_t(6) + gauge_bound_t(2, k)));

    test_gauge_semilattice(gauge_semilattice_t::bottom(), true, false);
    test_gauge_semilattice(gauge_semilattice_t::top(), false, true);
    test_gauge_semilattice(l1, false, false);
    test_gauge_semilattice(l2, false, false);
    test_gauge_semilattice(l3, false, false);
    test_gauge_semilattice(l4, false, false);
    test_gauge_semilattice(l5, false, false);

    std::cout
        << "# Test <= ####################################################"
        << std::endl;

    test_gauge_semilattice_le(gauge_semilattice_t::top(),
                              gauge_semilattice_t::top(),
                              true);
    test_gauge_semilattice_le(gauge_semilattice_t::top(),
                              gauge_semilattice_t::bottom(),
                              false);
    test_gauge_semilattice_le(gauge_semilattice_t::top(), l1, false);
    test_gauge_semilattice_le(gauge_semilattice_t::top(), l2, false);
    test_gauge_semilattice_le(gauge_semilattice_t::top(), l3, false);
    test_gauge_semilattice_le(gauge_semilattice_t::top(), l4, false);
    test_gauge_semilattice_le(gauge_semilattice_t::top(), l5, false);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(),
                              gauge_semilattice_t::top(),
                              true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(),
                              gauge_semilattice_t::bottom(),
                              true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(), l1, true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(), l2, true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(), l3, true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(), l4, true);
    test_gauge_semilattice_le(gauge_semilattice_t::bottom(), l5, true);
    test_gauge_semilattice_le(l1, gauge_semilattice_t::top(), true);
    test_gauge_semilattice_le(l1, gauge_semilattice_t::bottom(), false);
    test_gauge_semilattice_le(l1, l1, true);
    test_gauge_semilattice_le(l1, l2, false);
    test_gauge_semilattice_le(l1, l3, true);
    test_gauge_semilattice_le(l1, l4, false);
    test_gauge_semilattice_le(l1, l5, false);
    test_gauge_semilattice_le(l2, gauge_semilattice_t::top(), true);
    test_gauge_semilattice_le(l2, gauge_semilattice_t::bottom(), false);
    test_gauge_semilattice_le(l2, l1, true);
    test_gauge_semilattice_le(l2, l2, true);
    test_gauge_semilattice_le(l2, l3, true);
    test_gauge_semilattice_le(l2, l4, true);
    test_gauge_semilattice_le(l2, l5, false);
    test_gauge_semilattice_le(l3, gauge_semilattice_t::top(), true);
    test_gauge_semilattice_le(l3, gauge_semilattice_t::bottom(), false);
    test_gauge_semilattice_le(l3, l1, false);
    test_gauge_semilattice_le(l3, l2, false);
    test_gauge_semilattice_le(l3, l3, true);
    test_gauge_semilattice_le(l3, l4, false);
    test_gauge_semilattice_le(l3, l5, false);
    test_gauge_semilattice_le(l4, gauge_semilattice_t::top(), true);
    test_gauge_semilattice_le(l4, gauge_semilattice_t::bottom(), false);
    test_gauge_semilattice_le(l4, l1, false);
    test_gauge_semilattice_le(l4, l2, false);
    test_gauge_semilattice_le(l4, l3, true);
    test_gauge_semilattice_le(l4, l4, true);
    test_gauge_semilattice_le(l4, l5, false);
    test_gauge_semilattice_le(l5, gauge_semilattice_t::top(), true);
    test_gauge_semilattice_le(l5, gauge_semilattice_t::bottom(), false);
    test_gauge_semilattice_le(l5, l1, true);
    test_gauge_semilattice_le(l5, l2, false);
    test_gauge_semilattice_le(l5, l3, true);
    test_gauge_semilattice_le(l5, l4, false);
    test_gauge_semilattice_le(l5, l5, true);

    std::cout
        << "# Test join ##################################################"
        << std::endl;

    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                l1,
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                l2,
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                l3,
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                l4,
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::top(),
                                l5,
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(), l1, l1);
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(), l2, l2);
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(), l3, l3);
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(), l4, l4);
    test_gauge_semilattice_join(gauge_semilattice_t::bottom(), l5, l5);
    test_gauge_semilattice_join(l1,
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(l1, gauge_semilattice_t::bottom(), l1);
    test_gauge_semilattice_join(l1, l1, l1);
    test_gauge_semilattice_join(l1, l2, l1);
    test_gauge_semilattice_join(l1, l3, l3);
    test_gauge_semilattice_join(l1, l4, l3);
    test_gauge_semilattice_join(l1, l5, l1);
    test_gauge_semilattice_join(l2,
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(l2, gauge_semilattice_t::bottom(), l2);
    test_gauge_semilattice_join(l2, l1, l1);
    test_gauge_semilattice_join(l2, l2, l2);
    test_gauge_semilattice_join(l2, l3, l3);
    test_gauge_semilattice_join(l2, l4, l4);
    test_gauge_semilattice_join(l2, l5, l6);
    test_gauge_semilattice_join(l3,
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(l3, gauge_semilattice_t::bottom(), l3);
    test_gauge_semilattice_join(l3, l1, l3);
    test_gauge_semilattice_join(l3, l2, l3);
    test_gauge_semilattice_join(l3, l3, l3);
    test_gauge_semilattice_join(l3, l4, l3);
    test_gauge_semilattice_join(l3, l5, l3);
    test_gauge_semilattice_join(l4,
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(l4, gauge_semilattice_t::bottom(), l4);
    test_gauge_semilattice_join(l4, l1, l3);
    test_gauge_semilattice_join(l4, l2, l4);
    test_gauge_semilattice_join(l4, l3, l3);
    test_gauge_semilattice_join(l4, l4, l4);
    test_gauge_semilattice_join(l4, l5, l7);
    test_gauge_semilattice_join(l5,
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_join(l5, gauge_semilattice_t::bottom(), l5);
    test_gauge_semilattice_join(l5, l1, l1);
    test_gauge_semilattice_join(l5, l2, l6);
    test_gauge_semilattice_join(l5, l3, l3);
    test_gauge_semilattice_join(l5, l4, l7);
    test_gauge_semilattice_join(l5, l5, l5);

    std::cout
        << "# Test meet ##################################################"
        << std::endl;

    test_gauge_semilattice_meet(gauge_semilattice_t::top(),
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::top());
    test_gauge_semilattice_meet(gauge_semilattice_t::top(),
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::top(), l1, l1);
    test_gauge_semilattice_meet(gauge_semilattice_t::top(), l2, l2);
    test_gauge_semilattice_meet(gauge_semilattice_t::top(), l3, l3);
    test_gauge_semilattice_meet(gauge_semilattice_t::top(), l4, l4);
    test_gauge_semilattice_meet(gauge_semilattice_t::top(), l5, l5);
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::top(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                l1,
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                l2,
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                l3,
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                l4,
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(gauge_semilattice_t::bottom(),
                                l5,
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l1, gauge_semilattice_t::top(), l1);
    test_gauge_semilattice_meet(l1,
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l1, l1, l1);
    test_gauge_semilattice_meet(l1, l2, l2);
    test_gauge_semilattice_meet(l1, l3, l1);
    test_gauge_semilattice_meet(l1, l4, l8);
    test_gauge_semilattice_meet(l1, l5, l5);
    test_gauge_semilattice_meet(l2, gauge_semilattice_t::top(), l2);
    test_gauge_semilattice_meet(l2,
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l2, l1, l2);
    test_gauge_semilattice_meet(l2, l2, l2);
    test_gauge_semilattice_meet(l2, l3, l2);
    test_gauge_semilattice_meet(l2, l4, l2);
    test_gauge_semilattice_meet(l2, l5, l9);
    test_gauge_semilattice_meet(l3, gauge_semilattice_t::top(), l3);
    test_gauge_semilattice_meet(l3,
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l3, l1, l1);
    test_gauge_semilattice_meet(l3, l2, l2);
    test_gauge_semilattice_meet(l3, l3, l3);
    test_gauge_semilattice_meet(l3, l4, l4);
    test_gauge_semilattice_meet(l3, l5, l5);
    test_gauge_semilattice_meet(l4, gauge_semilattice_t::top(), l4);
    test_gauge_semilattice_meet(l4,
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l4, l1, l8);
    test_gauge_semilattice_meet(l4, l2, l2);
    test_gauge_semilattice_meet(l4, l3, l4);
    test_gauge_semilattice_meet(l4, l4, l4);
    test_gauge_semilattice_meet(l4, l5, l5);
    test_gauge_semilattice_meet(l5, gauge_semilattice_t::top(), l5);
    test_gauge_semilattice_meet(l5,
                                gauge_semilattice_t::bottom(),
                                gauge_semilattice_t::bottom());
    test_gauge_semilattice_meet(l5, l1, l5);
    test_gauge_semilattice_meet(l5, l2, l5);
    test_gauge_semilattice_meet(l5, l3, l5);
    test_gauge_semilattice_meet(l5, l4, l5);
    test_gauge_semilattice_meet(l5, l5, l5);

    std::cout
        << "# Test -= ####################################################"
        << std::endl;

    test_gauge_semilattice_forget(gauge_semilattice_t::top(),
                                  x,
                                  gauge_semilattice_t::top());
    test_gauge_semilattice_forget(gauge_semilattice_t::bottom(),
                                  x,
                                  gauge_semilattice_t::bottom());
    test_gauge_semilattice_forget(l1, x, gauge_semilattice_t::top());
    test_gauge_semilattice_forget(l1, y, l1);
    test_gauge_semilattice_forget(l2, y, l1);
    test_gauge_semilattice_forget(l3, x, gauge_semilattice_t::top());
    test_gauge_semilattice_forget(l4, y, l3);
    test_gauge_semilattice_forget(l5, y, l1);

    std::cout
        << "# Test incr counter ##########################################"
        << std::endl;

    test_gauge_semilattice_incr_counter(gauge_semilattice_t::top(),
                                        i,
                                        1,
                                        gauge_semilattice_t::top());
    test_gauge_semilattice_incr_counter(gauge_semilattice_t::bottom(),
                                        i,
                                        1,
                                        gauge_semilattice_t::bottom());

    // l1.incr_counter(i, 1)
    test_gauge_semilattice_incr_counter(l1, i, 1, l1);

    // l2.incr_counter(i, 1)
    gauge_semilattice_t l2_prime = l1;
    l2_prime.set(x, gauge_t(0, 1));
    l2_prime.set(y,
                 gauge_t(gauge_bound_t(-2) + gauge_bound_t(i),
                         gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_semilattice_incr_counter(l2, i, 1, l2_prime);

    // l3.incr_counter(i, 1)
    test_gauge_semilattice_incr_counter(l3, i, 1, l3);

    // l4.incr_counter(i, 1)
    gauge_semilattice_t l4_prime;
    l4_prime.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));
    l4_prime.set(y,
                 gauge_t(gauge_bound_t(-3) + gauge_bound_t(i),
                         gauge_bound_t(-3) + gauge_bound_t(3, i)));
    test_gauge_semilattice_incr_counter(l4, i, 1, l4_prime);

    // l5.incr_counter(i, 1)
    test_gauge_semilattice_incr_counter(l5, i, 1, l5);

    // l5.incr_counter(k, 2)
    gauge_semilattice_t l5_prime;
    l5_prime.set(x, gauge_t(0, 1));
    l5_prime.set(y,
                 gauge_t(gauge_bound_t(-4) + gauge_bound_t(2, k),
                         gauge_bound_t(-4) + gauge_bound_t(2, k)));
    test_gauge_semilattice_incr_counter(l5, k, 2, l5_prime);

    // l6.incr_counter(i, 1)
    gauge_semilattice_t l6_prime;
    l6_prime.set(x, gauge_t(0, 1));
    l6_prime.set(y, gauge_t(-2, gauge_bound_t(2, i) + gauge_bound_t(2, k)));
    test_gauge_semilattice_incr_counter(l6, i, 1, l6_prime);

    // l7.incr_counter(i, 1)
    gauge_semilattice_t l7_prime;
    l7_prime.set(x, gauge_t(0, gauge_bound_t::plus_infinity()));
    l7_prime.set(y,
                 gauge_t(-3,
                         gauge_bound_t(-2) + gauge_bound_t(3, i) +
                             gauge_bound_t(2, k)));
    test_gauge_semilattice_incr_counter(l7, i, 1, l7_prime);

    std::cout
        << "# Test forget counter ########################################"
        << std::endl;

    test_gauge_semilattice_forget_counter(gauge_semilattice_t::top(),
                                          x,
                                          gauge_semilattice_t::top());
    test_gauge_semilattice_forget_counter(gauge_semilattice_t::bottom(),
                                          x,
                                          gauge_semilattice_t::bottom());
    test_gauge_semilattice_forget_counter(l1, i, l1);
    test_gauge_semilattice_forget_counter(l2, i, l10);
    test_gauge_semilattice_forget_counter(l2, i, interval_t(1, 3), l12);
    test_gauge_semilattice_forget_counter(l3, i, l3);
    test_gauge_semilattice_forget_counter(l4, i, l11);
    test_gauge_semilattice_forget_counter(l5, i, l5);
    test_gauge_semilattice_forget_counter(l5, k, l10);
    test_gauge_semilattice_forget_counter(l6, i, l10);
    test_gauge_semilattice_forget_counter(l6, i, interval_t(1, 3), l13);
    test_gauge_semilattice_forget_counter(l6,
                                          i,
                                          interval_t(1,
                                                     bound_t::plus_infinity()),
                                          l10);
    test_gauge_semilattice_forget_counter(l6, k, l10);
    test_gauge_semilattice_forget_counter(l7, i, l11);
    test_gauge_semilattice_forget_counter(l7, k, l11);

    std::cout
        << "# Test operator[] ############################################"
        << std::endl;

    test_gauge_semilattice_get(gauge_semilattice_t::top(), x, gauge_t::top());
    test_gauge_semilattice_get(gauge_semilattice_t::bottom(),
                               x,
                               gauge_t::bottom());
    test_gauge_semilattice_get(l1, x, gauge_t(0, 1));
    test_gauge_semilattice_get(l1, y, gauge_t::top());
    test_gauge_semilattice_get(l2, x, gauge_t(0, 1));
    test_gauge_semilattice_get(l3,
                               x,
                               gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_semilattice_get(l4,
                               x,
                               gauge_t(0, gauge_bound_t::plus_infinity()));
    test_gauge_semilattice_get(l5, x, gauge_t(0, 1));

    std::cout
        << "# Test gauge domain ##########################################"
        << std::endl;

    gauge_domain_t d, d1, d2, d3, d4, d5, d6;

    test_gauge_domain(gauge_domain_t::bottom(), true, false);
    test_gauge_domain(gauge_domain_t::top(), false, true);

    d = gauge_domain_t::top();
    d.init_counter(i, z_number(0));
    test_gauge_domain(d, false, false);

    d = gauge_domain_t::top();
    d.init_counter(i, z_number(0));
    d.assign(x, 1);
    test_gauge_domain(d, false, false);

    std::cout
        << "# Test assign and operator[] #################################"
        << std::endl;

    test_gauge_domain_get(gauge_domain_t::bottom(), x, gauge_t::bottom());
    test_gauge_domain_to_interval(gauge_domain_t::bottom(),
                                  x,
                                  interval_t::bottom());
    test_gauge_domain_get(gauge_domain_t::top(), x, gauge_t::top());
    test_gauge_domain_to_interval(gauge_domain_t::top(), x, interval_t::top());

    d = gauge_domain_t::top();
    d.init_counter(i, z_number(0));
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(0, 0));
    test_gauge_domain_get(d, x, gauge_t::top());
    test_gauge_domain_to_interval(d, x, interval_t::top());

    d.assign(x, 1);
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(0, 0));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));

    d.assign(y, 2 * variable_t(i) + 1);
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(0, 0));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));
    test_gauge_domain_get(d,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d, y, interval_t(1, 1));

    d.incr_counter(i, z_number(1));
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(1, 1));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));
    test_gauge_domain_get(d,
                          y,
                          gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d, y, interval_t(1, 1));

    d.init_counter(k, z_number(0));
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(1, 1));
    test_gauge_domain_get(d, k, gauge_t(gauge_bound_t(k)));
    test_gauge_domain_to_interval(d, k, interval_t(0, 0));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));
    test_gauge_domain_get(d,
                          y,
                          gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d, y, interval_t(1, 1));

    d.assign(z, 3 * variable_t(k) + 2 * variable_t(y));
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(1, 1));
    test_gauge_domain_get(d, k, gauge_t(gauge_bound_t(k)));
    test_gauge_domain_to_interval(d, k, interval_t(0, 0));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));
    test_gauge_domain_get(d,
                          y,
                          gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d, y, interval_t(1, 1));
    test_gauge_domain_get(d,
                          z,
                          gauge_t(gauge_bound_t(-2) + gauge_bound_t(4, i) +
                                  gauge_bound_t(3, k)));
    test_gauge_domain_to_interval(d, z, interval_t(2, 2));

    d.incr_counter(k, z_number(1));
    test_gauge_domain_get(d, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d, i, interval_t(1, 1));
    test_gauge_domain_get(d, k, gauge_t(gauge_bound_t(k)));
    test_gauge_domain_to_interval(d, k, interval_t(1, 1));
    test_gauge_domain_get(d, x, gauge_t(1, 1));
    test_gauge_domain_to_interval(d, x, interval_t(1, 1));
    test_gauge_domain_get(d,
                          y,
                          gauge_t(gauge_bound_t(-1) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d, y, interval_t(1, 1));
    test_gauge_domain_get(d,
                          z,
                          gauge_t(gauge_bound_t(-5) + gauge_bound_t(4, i) +
                                  gauge_bound_t(3, k)));
    test_gauge_domain_to_interval(d, z, interval_t(2, 2));

    std::cout
        << "# Test widening ##############################################"
        << std::endl;

    d1 = gauge_domain_t::top();

    d1.init_counter(i, z_number(0));
    d1.assign(x, 0);
    d1.assign(y, 1);
    test_gauge_domain_get(d1, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d1, i, interval_t(0, 0));
    test_gauge_domain_get(d1, x, gauge_t(0, 0));
    test_gauge_domain_to_interval(d1, x, interval_t(0, 0));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));
    test_gauge_domain_to_interval(d1, y, interval_t(1, 1));

    d2 = d1;
    d2.assign(i, variable_t(i) + 1);
    d2.assign(x, variable_t(x) + 2);
    d2.assign(y, variable_t(y) + 3);
    test_gauge_domain_get(d2, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, i, interval_t(1, 1));
    test_gauge_domain_get(d2, x, gauge_t(2, 2));
    test_gauge_domain_to_interval(d2, x, interval_t(2, 2));
    test_gauge_domain_get(d2, y, gauge_t(4, 4));
    test_gauge_domain_to_interval(d2, y, interval_t(4, 4));

    d3 = d1 || d2;
    test_gauge_domain_get(d3, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d3,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d3, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3,
                                  x,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d3,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(3, i)));
    test_gauge_domain_to_interval(d3,
                                  y,
                                  interval_t(1, bound_t::plus_infinity()));

    d4 = d3;
    d4.assign(y, 3 * variable_t(i) + 2);

    d5 = d3 || d4;
    test_gauge_domain_get(d5, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d5,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d5,
                                  x,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(3, i),
                                  gauge_bound_t::plus_infinity()));
    test_gauge_domain_to_interval(d5,
                                  y,
                                  interval_t(1, bound_t::plus_infinity()));

    std::cout
        << "# Test assertion #############################################"
        << std::endl;

    d1 += (variable_t(i) <= 10);
    test_gauge_domain_get(d1, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d1, i, interval_t(0, 0));
    test_gauge_domain_get(d1, x, gauge_t(0, 0));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    d2 += (variable_t(i) <= 10);
    test_gauge_domain_get(d2, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, i, interval_t(1, 1));
    test_gauge_domain_get(d2, x, gauge_t(2, 2));
    test_gauge_domain_get(d2, y, gauge_t(4, 4));

    d3 += (variable_t(i) <= 10);
    test_gauge_domain_get(d3, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d3, i, interval_t(0, 10));
    test_gauge_domain_get(d3, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3, x, interval_t(0, 20));
    test_gauge_domain_get(d3,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(3, i)));
    test_gauge_domain_to_interval(d3, y, interval_t(1, 31));

    d4 = d3;
    d4 += (variable_t(i) >= 11);
    test_gauge_domain_get(d4, i, gauge_t::bottom());

    d5 = d3;
    d5 += (variable_t(i) == 10);
    test_gauge_domain_get(d5, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d5, i, interval_t(10, 10));
    test_gauge_domain_get(d5, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d5, x, interval_t(20, 20));
    test_gauge_domain_get(d5,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(3, i)));
    test_gauge_domain_to_interval(d5, y, interval_t(31, 31));

    d6 = d3;
    d6 += (variable_t(i) != 10);
    test_gauge_domain_get(d6, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d6, i, interval_t(0, 9));
    test_gauge_domain_get(d6, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d6, x, interval_t(0, 18));
    test_gauge_domain_get(d6,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(3, i)));
    test_gauge_domain_to_interval(d6, y, interval_t(1, 28));

    std::cout
        << "# Test forget ################################################"
        << std::endl;

    d1 -= i;
    test_gauge_domain_get(d1, i, gauge_t::top());
    test_gauge_domain_to_interval(d1, i, interval_t::top());
    test_gauge_domain_get(d1, x, gauge_t(0, 0));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    d3 -= y;
    test_gauge_domain_get(d3, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d3, i, interval_t(0, 10));
    test_gauge_domain_get(d3, x, gauge_t(gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3, x, interval_t(0, 20));
    test_gauge_domain_get(d3, y, gauge_t::top());
    test_gauge_domain_to_interval(d3, y, interval_t::top());

    d3 -= i;
    test_gauge_domain_get(d3, i, gauge_t::top());
    test_gauge_domain_to_interval(d3, i, interval_t::top());
    test_gauge_domain_get(d3, x, gauge_t(0, 20));
    test_gauge_domain_to_interval(d3, x, interval_t(0, 20));

    std::cout
        << "# Test set_interval ##########################################"
        << std::endl;

    d1.set_interval(x, interval_t::top());
    test_gauge_domain_get(d1, x, gauge_t::top());
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    d1.set_interval(x, interval_t(1, 2));
    test_gauge_domain_get(d1, x, gauge_t(1, 2));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    d1.set_interval(x, interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d1, x, gauge_t(1, gauge_bound_t::plus_infinity()));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    d1.set_interval(x, interval_t(bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d1, x, gauge_t(gauge_bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d1, y, gauge_t(1, 1));

    std::cout
        << "# Test apply(operation, var, var, var) #######################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.assign(zero, 0);
    d1.assign(one, 1);
    d1.assign(two, 2);
    d1.init_counter(i, z_number(0));
    d1.assign(x, 3);
    d1.assign(y, 2);
    d1.assign(z, 2);
    d1.assign(w, 16);
    test_gauge_domain_get(d1, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d1, i, interval_t(0, 0));
    test_gauge_domain_get(d1, x, gauge_t(3, 3));
    test_gauge_domain_to_interval(d1, x, interval_t(3, 3));
    test_gauge_domain_get(d1, y, gauge_t(2, 2));
    test_gauge_domain_to_interval(d1, y, interval_t(2, 2));
    test_gauge_domain_get(d1, z, gauge_t(2, 2));
    test_gauge_domain_to_interval(d1, z, interval_t(2, 2));
    test_gauge_domain_get(d1, w, gauge_t(16, 16));
    test_gauge_domain_to_interval(d1, w, interval_t(16, 16));

    d2 = d1;
    d2.apply(OP_ADDITION, x, x, two);
    d2.apply(OP_SUBTRACTION, y, y, one);
    d2.apply(OP_MULTIPLICATION, z, z, two);
    d2.apply(OP_DIVISION, w, w, two);
    d2.apply(OP_ADDITION, k, i, one);
    test_gauge_domain_get(d2, x, gauge_t(5, 5));
    test_gauge_domain_to_interval(d2, x, interval_t(5, 5));
    test_gauge_domain_get(d2, y, gauge_t(1, 1));
    test_gauge_domain_to_interval(d2, y, interval_t(1, 1));
    test_gauge_domain_get(d2, z, gauge_t(4, 4));
    test_gauge_domain_to_interval(d2, z, interval_t(4, 4));
    test_gauge_domain_get(d2, w, gauge_t(8, 8));
    test_gauge_domain_to_interval(d2, w, interval_t(8, 8));
    test_gauge_domain_get(d2, k, gauge_t(gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, k, interval_t(1, 1));

    d2.apply(OP_ADDITION, i, k, zero);
    test_gauge_domain_get(d2, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, i, interval_t(1, 1));
    test_gauge_domain_get(d2, k, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, k, interval_t(1, 1));

    d3 = d1 || d2;
    test_gauge_domain_get(d3, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d3,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d3, k, gauge_t::top());
    test_gauge_domain_to_interval(d3, k, interval_t::top());
    test_gauge_domain_get(d3,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3,
                                  x,
                                  interval_t(3, bound_t::plus_infinity()));
    test_gauge_domain_get(d3,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d3,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 2));
    test_gauge_domain_get(d3,
                          z,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3,
                                  z,
                                  interval_t(2, bound_t::plus_infinity()));
    test_gauge_domain_get(d3,
                          w,
                          gauge_t(gauge_bound_t(16) + gauge_bound_t(-8, i)));
    test_gauge_domain_to_interval(d3,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 16));

    d4 = d3;
    d4.apply(OP_ADDITION, x, x, two);
    d4.apply(OP_SUBTRACTION, y, y, one);
    d4.apply(OP_MULTIPLICATION, z, z, two);
    d4.apply(OP_DIVISION, w, w, two);
    d4.apply(OP_ADDITION, k, i, one);
    test_gauge_domain_get(d4, k, gauge_t(gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  k,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          x,
                          gauge_t(gauge_bound_t(5) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d4,
                                  x,
                                  interval_t(5, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d4,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d4,
                          z,
                          gauge_t(gauge_bound_t(4) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d4,
                                  z,
                                  interval_t(4, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, w, gauge_t(gauge_bound_t::minus_infinity(), 8));
    test_gauge_domain_to_interval(d4,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 8));

    d4.apply(OP_ADDITION, i, k, zero);
    test_gauge_domain_get(d4, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  i,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, k, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  k,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d4,
                                  x,
                                  interval_t(5, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d4,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d4,
                          z,
                          gauge_t(gauge_bound_t(0) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d4,
                                  z,
                                  interval_t(4, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, w, gauge_t(gauge_bound_t::minus_infinity(), 8));
    test_gauge_domain_to_interval(d4,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 8));

    d5 = d3 | d4;
    test_gauge_domain_get(d5, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d5,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5, k, gauge_t::top());
    test_gauge_domain_to_interval(d5, k, interval_t::top());
    test_gauge_domain_get(d5,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d5,
                                  x,
                                  interval_t(3, bound_t::plus_infinity()));
    test_gauge_domain_get(d5,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d5,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 2));
    test_gauge_domain_get(d5,
                          z,
                          gauge_t(gauge_bound_t(2, i),
                                  gauge_bound_t(2) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d5,
                                  z,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5, w, gauge_t(gauge_bound_t::minus_infinity(), 16));
    test_gauge_domain_to_interval(d5,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 16));

    std::cout
        << "# Test apply(operation, var, var, num) #######################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.init_counter(i, z_number(0));
    d1.assign(x, 3);
    d1.assign(y, 2);
    d1.assign(z, 2);
    d1.assign(w, 16);
    test_gauge_domain_get(d1, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d1, i, interval_t(0, 0));
    test_gauge_domain_get(d1, x, gauge_t(3, 3));
    test_gauge_domain_to_interval(d1, x, interval_t(3, 3));
    test_gauge_domain_get(d1, y, gauge_t(2, 2));
    test_gauge_domain_to_interval(d1, y, interval_t(2, 2));
    test_gauge_domain_get(d1, z, gauge_t(2, 2));
    test_gauge_domain_to_interval(d1, z, interval_t(2, 2));
    test_gauge_domain_get(d1, w, gauge_t(16, 16));
    test_gauge_domain_to_interval(d1, w, interval_t(16, 16));

    d2 = d1;
    d2.apply(OP_ADDITION, x, x, z_number(2));
    d2.apply(OP_SUBTRACTION, y, y, z_number(1));
    d2.apply(OP_MULTIPLICATION, z, z, z_number(2));
    d2.apply(OP_DIVISION, w, w, z_number(2));
    d2.apply(OP_ADDITION, k, i, z_number(1));
    test_gauge_domain_get(d2, x, gauge_t(5, 5));
    test_gauge_domain_to_interval(d2, x, interval_t(5, 5));
    test_gauge_domain_get(d2, y, gauge_t(1, 1));
    test_gauge_domain_to_interval(d2, y, interval_t(1, 1));
    test_gauge_domain_get(d2, z, gauge_t(4, 4));
    test_gauge_domain_to_interval(d2, z, interval_t(4, 4));
    test_gauge_domain_get(d2, w, gauge_t(8, 8));
    test_gauge_domain_to_interval(d2, w, interval_t(8, 8));
    test_gauge_domain_get(d2, k, gauge_t(gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, k, interval_t(1, 1));

    d2.apply(OP_ADDITION, i, k, z_number(0));
    test_gauge_domain_get(d2, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, i, interval_t(1, 1));
    test_gauge_domain_get(d2, k, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d2, k, interval_t(1, 1));

    d3 = d1 || d2;
    test_gauge_domain_get(d3, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d3,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d3, k, gauge_t::top());
    test_gauge_domain_to_interval(d3, k, interval_t::top());
    test_gauge_domain_get(d3,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3,
                                  x,
                                  interval_t(3, bound_t::plus_infinity()));
    test_gauge_domain_get(d3,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d3,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 2));
    test_gauge_domain_get(d3,
                          z,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d3,
                                  z,
                                  interval_t(2, bound_t::plus_infinity()));
    test_gauge_domain_get(d3,
                          w,
                          gauge_t(gauge_bound_t(16) + gauge_bound_t(-8, i)));
    test_gauge_domain_to_interval(d3,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 16));

    d4 = d3;
    d4.apply(OP_ADDITION, x, x, z_number(2));
    d4.apply(OP_SUBTRACTION, y, y, z_number(1));
    d4.apply(OP_MULTIPLICATION, z, z, z_number(2));
    d4.apply(OP_DIVISION, w, w, z_number(2));
    d4.apply(OP_ADDITION, k, i, z_number(1));
    test_gauge_domain_get(d4, k, gauge_t(gauge_bound_t(1) + gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  k,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          x,
                          gauge_t(gauge_bound_t(5) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d4,
                                  x,
                                  interval_t(5, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          y,
                          gauge_t(gauge_bound_t(1) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d4,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d4,
                          z,
                          gauge_t(gauge_bound_t(4) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d4,
                                  z,
                                  interval_t(4, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, w, gauge_t(gauge_bound_t::minus_infinity(), 8));
    test_gauge_domain_to_interval(d4,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 8));

    d4.apply(OP_ADDITION, i, k, z_number(0));
    test_gauge_domain_get(d4, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  i,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, k, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d4,
                                  k,
                                  interval_t(1, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d4,
                                  x,
                                  interval_t(5, bound_t::plus_infinity()));
    test_gauge_domain_get(d4,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d4,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 1));
    test_gauge_domain_get(d4,
                          z,
                          gauge_t(gauge_bound_t(0) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d4,
                                  z,
                                  interval_t(4, bound_t::plus_infinity()));
    test_gauge_domain_get(d4, w, gauge_t(gauge_bound_t::minus_infinity(), 8));
    test_gauge_domain_to_interval(d4,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 8));

    d5 = d3 | d4;
    test_gauge_domain_get(d5, i, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_to_interval(d5,
                                  i,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5, k, gauge_t::top());
    test_gauge_domain_to_interval(d5, k, interval_t::top());
    test_gauge_domain_get(d5,
                          x,
                          gauge_t(gauge_bound_t(3) + gauge_bound_t(2, i)));
    test_gauge_domain_to_interval(d5,
                                  x,
                                  interval_t(3, bound_t::plus_infinity()));
    test_gauge_domain_get(d5,
                          y,
                          gauge_t(gauge_bound_t(2) + gauge_bound_t(-1, i)));
    test_gauge_domain_to_interval(d5,
                                  y,
                                  interval_t(bound_t::minus_infinity(), 2));
    test_gauge_domain_get(d5,
                          z,
                          gauge_t(gauge_bound_t(2, i),
                                  gauge_bound_t(2) + gauge_bound_t(4, i)));
    test_gauge_domain_to_interval(d5,
                                  z,
                                  interval_t(0, bound_t::plus_infinity()));
    test_gauge_domain_get(d5, w, gauge_t(gauge_bound_t::minus_infinity(), 16));
    test_gauge_domain_to_interval(d5,
                                  w,
                                  interval_t(bound_t::minus_infinity(), 16));

    std::cout
        << "# Test apply(conv_op, var, var) ##############################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.init_counter(i, z_number(0));
    d1.apply(OP_TRUNC, x, i, 32, 8);
    d1.apply(OP_ZEXT, y, i, 8, 32);
    d1.apply(OP_SEXT, z, i, 8, 32);
    test_gauge_domain_get(d1, x, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_get(d1, y, gauge_t(gauge_bound_t(i)));
    test_gauge_domain_get(d1, z, gauge_t(gauge_bound_t(i)));

    d2 = d1;
    d2.apply(OP_ADDITION, i, i, z_number(1));
    d3 = d1 || d2;
    d3.apply(OP_TRUNC, x, i, 32, 8);
    d3.apply(OP_ZEXT, y, i, 8, 32);
    d3.apply(OP_SEXT, z, i, 8, 32);
    test_gauge_domain_get(d3, x, gauge_t(-128, 255));
    test_gauge_domain_get(d3, y, gauge_t(0, 255));
    test_gauge_domain_get(d3, z, gauge_t(-128, 255));

    std::cout
        << "# Test apply(conv_op, var, num) ##############################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.apply(OP_TRUNC, x, z_number(0x101), 32, 8);
    d1.apply(OP_ZEXT, y, z_number(0xff), 8, 32);
    d1.apply(OP_SEXT, z, z_number(-1), 8, 32);
    test_gauge_domain_get(d1, x, gauge_t(1, 1));
    test_gauge_domain_get(d1, y, gauge_t(0xff, 0xff));
    test_gauge_domain_get(d1, z, gauge_t(-1, -1));

    std::cout
        << "# Test apply(bitwise_op, var, var, var) ######################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.assign(x, 0b0110);
    d1.assign(y, 0b1010);
    d1.apply(OP_AND, z, x, y);
    d1.apply(OP_OR, w, x, y);
    d1.apply(OP_XOR, i, x, y);
    test_gauge_domain_get(d1, z, gauge_t(0b0010, 0b0010));
    test_gauge_domain_get(d1, w, gauge_t(0b1110, 0b1110));
    test_gauge_domain_get(d1, i, gauge_t(0b1100, 0b1100));

    std::cout
        << "# Test apply(bitwise_op, var, var, num) ######################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.assign(x, 0b0110);
    d1.apply(OP_AND, z, x, z_number(0b1010));
    d1.apply(OP_OR, w, x, z_number(0b1010));
    d1.apply(OP_XOR, i, x, z_number(0b1010));
    test_gauge_domain_get(d1, z, gauge_t(0b0010, 0b0010));
    test_gauge_domain_get(d1, w, gauge_t(0b1110, 0b1110));
    test_gauge_domain_get(d1, i, gauge_t(0b1100, 0b1100));

    std::cout
        << "# Test apply(div_op, var, var, var) ##########################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.assign(x, 13);
    d1.assign(y, 5);
    d1.apply(OP_SDIV, z, x, y);
    d1.apply(OP_SREM, w, x, y);
    test_gauge_domain_get(d1, z, gauge_t(2, 2));
    test_gauge_domain_get(d1, w, gauge_t(3, 3));

    std::cout
        << "# Test apply(div_op, var, var, num) ##########################"
        << std::endl;

    d1 = gauge_domain_t::top();
    d1.assign(x, 13);
    d1.apply(OP_SDIV, z, x, z_number(5));
    d1.apply(OP_SREM, w, x, z_number(5));
    test_gauge_domain_get(d1, z, gauge_t(2, 2));
    test_gauge_domain_get(d1, w, gauge_t(3, 3));
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  end_tests();
}
