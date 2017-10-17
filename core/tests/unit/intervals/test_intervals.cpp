#include <iostream>

#include <ikos/domains/intervals.hpp>

using namespace ikos;

typedef bound< z_number > bound_t;
typedef interval< z_number > interval_t;

void test_srem(interval_t left, interval_t right) {
  interval_t result = left.SRem(right);
  std::cout << "srem " << left << " % " << right << " = " << result
            << std::endl;
}

void test_urem(interval_t left, interval_t right) {
  interval_t result = left.URem(right);
  std::cout << "urem " << left << " % " << right << " = " << result
            << std::endl;
}

void test_trunc(interval_t op, uint64_t from, uint64_t to) {
  interval_t result = op.Trunc(from, to);
  std::cout << "trunc " << op << " from i" << from << " to i" << to << " = "
            << result << std::endl;
}

void test_zext(interval_t op, uint64_t from, uint64_t to) {
  interval_t result = op.ZExt(from, to);
  std::cout << "zext " << op << " from i" << from << " to i" << to << " = "
            << result << std::endl;
}

void test_sext(interval_t op, uint64_t from, uint64_t to) {
  interval_t result = op.SExt(from, to);
  std::cout << "sext " << op << " from i" << from << " to i" << to << " = "
            << result << std::endl;
}

void test_and(interval_t left, interval_t right) {
  interval_t result = left.And(right);
  std::cout << left << " & " << right << " = " << result << std::endl;
}

void test_or(interval_t left, interval_t right) {
  interval_t result = left.Or(right);
  std::cout << left << " | " << right << " = " << result << std::endl;
}

void test_xor(interval_t left, interval_t right) {
  interval_t result = left.Xor(right);
  std::cout << left << " ^ " << right << " = " << result << std::endl;
}

void test_shl(interval_t left, interval_t right) {
  interval_t result = left.Shl(right);
  std::cout << left << " << " << right << " = " << result << std::endl;
}

void test_lshr(interval_t left, interval_t right) {
  interval_t result = left.LShr(right);
  std::cout << left << " >> " << right << " = " << result << std::endl;
}

int main(int argc, char** argv) {
  try {
    interval_t interval = interval_t::top();

    std::cout
        << "# Test z_number::SRem() ######################################"
        << std::endl;
    test_srem(interval_t(z_number(9)), interval_t(z_number(4)));
    test_srem(interval_t(z_number(9)), interval_t(z_number(-4)));
    test_srem(interval_t(z_number(-9)), interval_t(z_number(-4)));
    test_srem(interval_t(z_number(-9)), interval_t(z_number(4)));
    test_srem(interval_t(z_number(9)), interval_t(z_number(0)));
    test_srem(interval_t(1, 42), interval_t(z_number(4)));
    test_srem(interval_t(-69, 42), interval_t(z_number(4)));
    test_srem(interval_t(-69, 0), interval_t(z_number(4)));
    test_srem(interval_t(1, 42), interval_t(-2, 4));
    test_srem(interval_t(-69, 42), interval_t(-2, 4));
    test_srem(interval_t(-69, 0), interval_t(-2, 4));
    test_srem(interval_t(1, 42), interval_t(-6, 4));
    test_srem(interval_t(-69, 42), interval_t(-6, 4));
    test_srem(interval_t(-69, 0), interval_t(-6, 4));
    test_srem(interval_t(-69, 42), interval_t(z_number(0)));

    std::cout
        << "# Test z_number::URem() ######################################"
        << std::endl;
    test_urem(interval_t(z_number(9)), interval_t(z_number(4)));
    test_urem(interval_t(z_number(9)), interval_t(z_number(-4)));
    test_urem(interval_t(z_number(-9)), interval_t(z_number(-4)));
    test_urem(interval_t(z_number(-9)), interval_t(z_number(4)));
    test_urem(interval_t(z_number(9)), interval_t(z_number(0)));
    test_urem(interval_t(1, 42), interval_t(z_number(4)));
    test_urem(interval_t(-69, 42), interval_t(z_number(4)));
    test_urem(interval_t(-69, 0), interval_t(z_number(4)));
    test_urem(interval_t(1, 42), interval_t(-2, 4));
    test_urem(interval_t(-69, 42), interval_t(-2, 4));
    test_urem(interval_t(-69, 0), interval_t(-2, 4));
    test_urem(interval_t(1, 42), interval_t(-6, 4));
    test_urem(interval_t(-69, 42), interval_t(-6, 4));
    test_urem(interval_t(-69, 0), interval_t(-6, 4));
    test_urem(interval_t(-69, 42), interval_t(z_number(0)));

    std::cout
        << "# Test z_number::And() #######################################"
        << std::endl;
    test_and(interval_t(z_number(202)), interval_t(z_number(185)));
    test_and(interval_t(-42, 69), interval_t(z_number(13)));
    test_and(interval_t(z_number(13)), interval_t(-42, 69));
    test_and(interval_t(9, 42), interval_t(-42, 69));
    test_and(interval_t(-9, 42), interval_t(-42, 69));
    test_and(interval_t::top(), interval_t(42, 69));
    test_and(interval_t::top(), interval_t(-42, 69));

    std::cout
        << "# Test z_number::Or() ########################################"
        << std::endl;
    test_or(interval_t(z_number(202)), interval_t(z_number(185)));
    test_or(interval_t(42, 69), interval_t(z_number(13)));
    test_or(interval_t(z_number(13)), interval_t(42, 69));
    test_or(interval_t(2, 13), interval_t(42, 69));
    test_or(interval_t(9, 42), interval_t(-42, 69));
    test_or(interval_t::top(), interval_t(42, 69));
    test_or(interval_t(0, bound_t::plus_infinity()), interval_t(42, 69));

    std::cout
        << "# Test z_number::Xor() ########################################"
        << std::endl;
    test_xor(interval_t(z_number(202)), interval_t(z_number(185)));
    test_xor(interval_t(42, 69), interval_t(z_number(13)));
    test_xor(interval_t(z_number(13)), interval_t(42, 69));
    test_xor(interval_t(2, 13), interval_t(42, 69));
    test_xor(interval_t(9, 42), interval_t(-42, 69));
    test_xor(interval_t::top(), interval_t(42, 69));
    test_xor(interval_t(0, bound_t::plus_infinity()), interval_t(42, 69));

    std::cout
        << "# Test z_number::Shl() #######################################"
        << std::endl;
    test_shl(interval_t(z_number(6)), interval_t(z_number(2)));
    test_shl(interval_t(4, 6), interval_t(z_number(2)));
    test_shl(interval_t(4, 84), interval_t(2, 4));
    test_shl(interval_t(4, 84), interval_t(0, 4));
    test_shl(interval_t(4, 84), interval_t(bound_t::minus_infinity(), 4));
    test_shl(interval_t(-4, 84), interval_t(2, 4));
    test_shl(interval_t(bound_t::minus_infinity(), 84), interval_t(2, 4));
    test_shl(interval_t(4, bound_t::plus_infinity()), interval_t(2, 4));
    test_shl(interval_t(4, 84), interval_t(2, bound_t::plus_infinity()));

    std::cout
        << "# Test z_number::LShr() ######################################"
        << std::endl;
    test_lshr(interval_t(z_number(6)), interval_t(z_number(2)));
    test_lshr(interval_t(4, 6), interval_t(z_number(2)));
    test_lshr(interval_t(4, 84), interval_t(2, 4));
    test_lshr(interval_t(4, 84), interval_t(0, 4));
    test_lshr(interval_t(4, 84), interval_t(bound_t::minus_infinity(), 4));
    test_lshr(interval_t(-4, 84), interval_t(2, 4));
    test_lshr(interval_t(bound_t::minus_infinity(), 84), interval_t(2, 4));
    test_lshr(interval_t(4, bound_t::plus_infinity()), interval_t(2, 4));
    test_lshr(interval_t(4, 84), interval_t(2, bound_t::plus_infinity()));

    std::cout
        << "# Test z_number::Trunc() ######################################"
        << std::endl;
    test_trunc(interval_t(z_number(42)), 32, 8);
    test_trunc(interval_t(z_number(0xffffff00)), 32, 8);
    test_trunc(interval_t(z_number(-2147483520)), 32, 8);
    test_trunc(interval_t(z_number(-2147483648)), 32, 8);
    test_trunc(interval_t(-42, 42), 32, 8);
    test_trunc(interval_t(-129, 42), 32, 8);
    test_trunc(interval_t(-42, 256), 32, 8);
    test_trunc(interval_t(-2147483520, -2147483519), 32, 8);
    test_trunc(interval_t(-2, bound_t::plus_infinity()), 32, 8);
    test_trunc(interval_t(2, bound_t::plus_infinity()), 32, 8);
    test_trunc(interval_t(bound_t::minus_infinity(), -2), 32, 8);
    test_trunc(interval_t(bound_t::minus_infinity(), 2), 32, 8);

    std::cout
        << "# Test z_number::ZExt() ######################################"
        << std::endl;
    test_zext(interval_t(z_number(42)), 8, 32);
    test_zext(interval_t(z_number(-1)), 8, 32);
    test_zext(interval_t(0, 42), 8, 32);
    test_zext(interval_t(-42, 42), 8, 32);
    test_zext(interval_t(-2, -1), 8, 32);
    test_zext(interval_t(2, bound_t::plus_infinity()), 8, 32);
    test_zext(interval_t(-2, bound_t::plus_infinity()), 8, 32);
    test_zext(interval_t(bound_t::minus_infinity(), -2), 8, 32);
    test_zext(interval_t(bound_t::minus_infinity(), 2), 8, 32);

    std::cout
        << "# Test z_number::SExt() ######################################"
        << std::endl;
    test_sext(interval_t(z_number(42)), 8, 32);
    test_sext(interval_t(z_number(-1)), 8, 32);
    test_sext(interval_t(0, 42), 8, 32);
    test_sext(interval_t(-42, 42), 8, 32);
    test_sext(interval_t(-2, -1), 8, 32);
    test_sext(interval_t(2, bound_t::plus_infinity()), 8, 32);
    test_sext(interval_t(-2, bound_t::plus_infinity()), 8, 32);
    test_sext(interval_t(bound_t::minus_infinity(), -2), 8, 32);
    test_sext(interval_t(bound_t::minus_infinity(), 2), 8, 32);
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
