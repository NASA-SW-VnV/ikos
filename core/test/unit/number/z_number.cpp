/*******************************************************************************
 *
 * Tests for ZNumber
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#include <limits>

#define BOOST_TEST_MODULE test_z_number
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION >= 107100
#include <boost/test/tools/output_test_stream.hpp>
#else
#include <boost/test/output_test_stream.hpp>
#endif

#include <ikos/core/number/z_number.hpp>

BOOST_AUTO_TEST_CASE(test_z_number) {
  using Z = ikos::core::ZNumber;
  using NumberError = ikos::core::NumberError;

  // Assume (unsigned) long long is 8 bytes
  // NOLINTNEXTLINE(google-runtime-int)
  static_assert(sizeof(long long) == 8, "unexpected size");
  // NOLINTNEXTLINE(google-runtime-int)
  static_assert(sizeof(unsigned long long) == 8, "unexpected size");

  // constructors
  BOOST_CHECK((Z() == Z(0)));
  BOOST_CHECK((Z(1) == Z(static_cast< int >(1))));
  BOOST_CHECK((Z(-1) == Z(static_cast< int >(-1))));
  BOOST_CHECK((Z(1) == Z(static_cast< unsigned int >(1))));
  BOOST_CHECK((Z(1) == Z(static_cast< long >(1))));
  BOOST_CHECK((Z(-1) == Z(static_cast< long >(-1))));
  BOOST_CHECK((Z(1) == Z(static_cast< unsigned long >(1))));
  BOOST_CHECK((Z(1) == Z(static_cast< long long >(1))));
  BOOST_CHECK((Z(-1) == Z(static_cast< long long >(-1))));
  BOOST_CHECK((-(Z(1) << 63) == Z(std::numeric_limits< long long >::min())));
  BOOST_CHECK((Z(1) << 63) - 1 == Z(std::numeric_limits< long long >::max()));
  BOOST_CHECK((Z(0x12345678) << 32) + Z(0x01234567) == Z(0x1234567801234567LL));
  BOOST_CHECK((Z(1) == Z(static_cast< unsigned long long >(1))));
  BOOST_CHECK((Z(1) << 64) - 1 ==
              Z(std::numeric_limits< unsigned long long >::max()));
  BOOST_CHECK((Z(0x12345678) << 32) + Z(0x01234567) ==
              Z(0x1234567801234567ULL));

  // operator=
  Z n;
  BOOST_CHECK((Z(1) == (n = static_cast< int >(1))));
  BOOST_CHECK((Z(-1) == (n = static_cast< int >(-1))));
  BOOST_CHECK((Z(1) == (n = static_cast< unsigned int >(1))));
  BOOST_CHECK((Z(1) == (n = static_cast< long >(1))));
  BOOST_CHECK((Z(-1) == (n = static_cast< long >(-1))));
  BOOST_CHECK((Z(1) == (n = static_cast< unsigned long >(1))));
  BOOST_CHECK((Z(1) == (n = static_cast< long long >(1))));
  BOOST_CHECK((Z(-1) == (n = static_cast< long long >(-1))));
  BOOST_CHECK((Z(1) == (n = static_cast< unsigned long long >(1))));

  // from_string
  BOOST_CHECK((Z(1) == Z::from_string("1")));
  BOOST_CHECK((Z(-1) == Z::from_string("-1")));
  BOOST_CHECK((Z(1) == Z::from_string("01")));
  BOOST_CHECK((Z(1) == Z::from_string(" 1")));
  BOOST_CHECK((Z(1) == Z::from_string("1\n")));
  BOOST_CHECK((Z(101) == Z::from_string("1 0 1")));
  BOOST_CHECK((Z(1) == Z::from_string("1", 10)));
  BOOST_CHECK((Z(2) == Z::from_string("10", 2)));
  BOOST_CHECK((Z(3) == Z::from_string("11", 2)));
  BOOST_CHECK((Z(10) == Z::from_string("A", 16)));
  BOOST_CHECK((Z(10) == Z::from_string("a", 16)));
  BOOST_CHECK((Z(0xFF) == Z::from_string("FF", 16)));
  BOOST_CHECK_THROW((Z::from_string("a")), NumberError);
  BOOST_CHECK_THROW((Z::from_string("12a")), NumberError);
  BOOST_CHECK_THROW((Z::from_string("a12")), NumberError);
  BOOST_CHECK_THROW((Z::from_string("0.12")), NumberError);
  BOOST_CHECK_THROW((Z::from_string("12\na")), NumberError);

  // operator+
  BOOST_CHECK((Z(1) + Z(2) == Z(3)));
  BOOST_CHECK((Z(1) + 2 == Z(3)));
  BOOST_CHECK((1 + Z(2) == Z(3)));
  n = 1;
  BOOST_CHECK((Z(3) == (n += Z(2))));
  n = 1;
  BOOST_CHECK((Z(3) == (n += 2)));

  BOOST_CHECK((Z(1) + Z(-2) == Z(-1)));
  BOOST_CHECK((Z(-1) + Z(2) == Z(1)));
  BOOST_CHECK((Z(-1) + Z(-2) == Z(-3)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) +
                   Z::from_string("deadbeefbadf00d", 16) ==
               Z::from_string("deadccda96c950bde1db", 16)));

  // operator-
  BOOST_CHECK((Z(3) - Z(2) == Z(1)));
  BOOST_CHECK((Z(3) - 2 == Z(1)));
  BOOST_CHECK((3 - Z(2) == Z(1)));
  n = 3;
  BOOST_CHECK((Z(1) == (n -= Z(2))));
  n = 3;
  BOOST_CHECK((Z(1) == (n -= 2)));

  BOOST_CHECK((Z(-3) - Z(1) == Z(-4)));
  BOOST_CHECK((Z(3) - Z(-1) == Z(4)));
  BOOST_CHECK((Z(-2) - Z(-3) == Z(1)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) -
                   Z::from_string("deadbeefbadf00d", 16) ==
               Z::from_string("deadb104deeb596201c1", 16)));

  // operator*
  BOOST_CHECK((Z(3) * Z(2) == Z(6)));
  BOOST_CHECK((Z(3) * 2 == Z(6)));
  BOOST_CHECK((3 * Z(2) == Z(6)));
  n = 3;
  BOOST_CHECK((Z(6) == (n *= Z(2))));
  n = 3;
  BOOST_CHECK((Z(6) == (n *= 2)));

  BOOST_CHECK((Z(3) * Z(-2) == Z(-6)));
  BOOST_CHECK((Z(-3) * Z(2) == Z(-6)));
  BOOST_CHECK((Z(-3) * Z(-2) == Z(6)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) *
                   Z::from_string("deadbeefbadf00d", 16) ==
               Z::from_string("c1b1cd13668200953a2fd455234a6b66776", 16)));

  // operator/
  BOOST_CHECK((Z(3) / Z(2) == Z(1)));
  BOOST_CHECK((Z(3) / 2 == Z(1)));
  BOOST_CHECK((3 / Z(2) == Z(1)));
  n = 3;
  BOOST_CHECK((Z(1) == (n /= Z(2))));
  n = 3;
  BOOST_CHECK((Z(1) == (n /= 2)));

  BOOST_CHECK((Z(11) / Z(3) == Z(3)));
  BOOST_CHECK((Z(11) / Z(-3) == Z(-3)));
  BOOST_CHECK((Z(-11) / Z(3) == Z(-3)));
  BOOST_CHECK((Z(-11) / Z(-3) == Z(3)));

  BOOST_CHECK((Z(12) / Z(3) == Z(4)));
  BOOST_CHECK((Z(12) / Z(-3) == Z(-4)));
  BOOST_CHECK((Z(-12) / Z(3) == Z(-4)));
  BOOST_CHECK((Z(-12) / Z(-3) == Z(4)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) /
                   Z::from_string("deadbeefbadf00d", 16) ==
               Z(0xfffff)));

  // operator%
  BOOST_CHECK((Z(3) % Z(2) == Z(1)));
  BOOST_CHECK((Z(3) % 2 == Z(1)));
  BOOST_CHECK((3 % Z(2) == Z(1)));
  n = 3;
  BOOST_CHECK((Z(1) == (n %= Z(2))));
  n = 3;
  BOOST_CHECK((Z(1) == (n %= 2)));

  BOOST_CHECK((Z(11) % Z(3) == Z(2)));
  BOOST_CHECK((Z(11) % Z(-3) == Z(2)));
  BOOST_CHECK((Z(-11) % Z(3) == Z(-2)));
  BOOST_CHECK((Z(-11) % Z(-3) == Z(-2)));

  BOOST_CHECK((Z(12) % Z(3) == Z(0)));
  BOOST_CHECK((Z(12) % Z(-3) == Z(0)));
  BOOST_CHECK((Z(-12) % Z(3) == Z(0)));
  BOOST_CHECK((Z(-12) % Z(-3) == Z(0)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) %
                   Z::from_string("deadbeefbadf00d", 16) ==
               Z::from_string("deadbea4fede1db", 16)));

  // operator&
  BOOST_CHECK(((Z(12) & Z(10)) == Z(8)));
  BOOST_CHECK(((Z(12) & 10) == Z(8)));
  BOOST_CHECK(((12 & Z(10)) == Z(8)));
  n = 12;
  BOOST_CHECK((Z(8) == (n &= Z(10))));
  n = 12;
  BOOST_CHECK((Z(8) == (n &= 10)));

  BOOST_CHECK(((Z(12) & Z(10)) == Z(8)));
  BOOST_CHECK(((Z(12) & Z(-10)) == Z(4)));
  BOOST_CHECK(((Z(-12) & Z(10)) == Z(0)));
  BOOST_CHECK(((Z(-12) & Z(-10)) == Z(-12)));

  BOOST_CHECK(((Z::from_string("deadbeefbada550ff1ce", 16) &
                Z::from_string("deadbeefbadf00d", 16)) ==
               Z::from_string("cea9aca510df00c", 16)));

  // operator|
  BOOST_CHECK(((Z(12) | Z(10)) == Z(14)));
  BOOST_CHECK(((Z(12) | 10) == Z(14)));
  BOOST_CHECK(((12 | Z(10)) == Z(14)));
  n = 14;
  BOOST_CHECK((Z(14) == (n |= Z(10))));
  n = 12;
  BOOST_CHECK((Z(14) == (n |= 10)));

  BOOST_CHECK(((Z(12) | Z(10)) == Z(14)));
  BOOST_CHECK(((Z(12) | Z(-10)) == Z(-2)));
  BOOST_CHECK(((Z(-12) | Z(10)) == Z(-2)));
  BOOST_CHECK(((Z(-12) | Z(-10)) == Z(-10)));

  BOOST_CHECK(((Z::from_string("deadbeefbada550ff1ce", 16) |
                Z::from_string("deadbeefbadf00d", 16)) ==
               Z::from_string("deadbfeffbfeffaff1cf", 16)));

  // operator^
  BOOST_CHECK(((Z(12) ^ Z(10)) == Z(6)));
  BOOST_CHECK(((Z(12) ^ 10) == Z(6)));
  BOOST_CHECK(((12 ^ Z(10)) == Z(6)));
  n = 12;
  BOOST_CHECK((Z(6) == (n ^= Z(10))));
  n = 12;
  BOOST_CHECK((Z(6) == (n ^= 10)));

  BOOST_CHECK(((Z(12) ^ Z(10)) == Z(6)));
  BOOST_CHECK(((Z(12) ^ Z(-10)) == Z(-6)));
  BOOST_CHECK(((Z(-12) ^ Z(10)) == Z(-2)));
  BOOST_CHECK(((Z(-12) ^ Z(-10)) == Z(2)));

  BOOST_CHECK(((Z::from_string("deadbeefbada550ff1ce", 16) ^
                Z::from_string("deadbeefbadf00d", 16)) ==
               Z::from_string("deadb3056134aea201c3", 16)));

  // operator<<
  BOOST_CHECK(((Z(10) << Z(2)) == Z(40)));
  BOOST_CHECK(((Z(10) << 2) == Z(40)));
  n = 10;
  BOOST_CHECK((Z(40) == (n <<= Z(2))));
  n = 10;
  BOOST_CHECK((Z(40) == (n <<= 2)));

  BOOST_CHECK(((Z(10) << Z(2)) == Z(40)));
  BOOST_CHECK(((Z(-10) << Z(2)) == Z(-40)));

  BOOST_CHECK(((Z::from_string("deadbeefbada550ff1ce", 16) << 42) ==
               Z::from_string("37ab6fbbeeb69543fc7380000000000", 16)));

  // operator>>
  BOOST_CHECK(((Z(10) >> Z(2)) == Z(2)));
  BOOST_CHECK(((Z(10) >> 2) == Z(2)));
  n = 10;
  BOOST_CHECK((Z(2) == (n >>= Z(2))));
  n = 10;
  BOOST_CHECK((Z(2) == (n >>= 2)));

  BOOST_CHECK(((Z(10) >> Z(2)) == Z(2)));
  BOOST_CHECK(((Z(-10) >> Z(2)) == Z(-3)));

  BOOST_CHECK(((Z::from_string("deadbeefbada550ff1ce", 16) >> 42) ==
               Z::from_string("37ab6fbbee", 16)));

  // operator++
  n = 0;
  BOOST_CHECK((n++ == Z(0)));
  n = 0;
  BOOST_CHECK((++n == Z(1)));

  // operator--
  n = 10;
  BOOST_CHECK((n-- == Z(10)));
  n = 10;
  BOOST_CHECK((--n == Z(9)));

  // operator==
  BOOST_CHECK((Z(1) == Z(1)));
  BOOST_CHECK((Z(1) == 1));
  BOOST_CHECK((1 == Z(1)));

  BOOST_CHECK((Z(1) == Z(1)));
  BOOST_CHECK((!(Z(1) == Z(2))));

  // operator!=
  BOOST_CHECK((Z(1) != Z(2)));
  BOOST_CHECK((Z(1) != 2));
  BOOST_CHECK((1 != Z(2)));

  BOOST_CHECK((Z(1) != Z(2)));
  BOOST_CHECK((!(Z(1) != Z(1))));

  // operator<
  BOOST_CHECK((Z(1) < Z(2)));
  BOOST_CHECK((Z(1) < 2));
  BOOST_CHECK((1 < Z(2)));

  BOOST_CHECK((Z(1) < Z(2)));
  BOOST_CHECK((!(Z(2) < Z(1))));
  BOOST_CHECK((!(Z(1) < Z(1))));

  BOOST_CHECK((Z::from_string("deadbeefbadf00d", 16) <
               Z::from_string("deadbeefbada550ff1ce", 16)));

  // operator<=
  BOOST_CHECK((Z(1) <= Z(2)));
  BOOST_CHECK((Z(1) <= 2));
  BOOST_CHECK((1 <= Z(2)));

  BOOST_CHECK((Z(1) <= Z(2)));
  BOOST_CHECK((!(Z(2) <= Z(1))));
  BOOST_CHECK((Z(1) <= Z(1)));

  BOOST_CHECK((Z::from_string("deadbeefbadf00d", 16) <=
               Z::from_string("deadbeefbada550ff1ce", 16)));

  // operator>
  BOOST_CHECK((Z(2) > Z(1)));
  BOOST_CHECK((Z(2) > 1));
  BOOST_CHECK((2 > Z(1)));

  BOOST_CHECK((Z(2) > Z(1)));
  BOOST_CHECK((!(Z(1) > Z(2))));
  BOOST_CHECK((!(Z(1) > Z(1))));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) >
               Z::from_string("deadbeefbadf00d", 16)));

  // operator>=
  BOOST_CHECK((Z(2) >= Z(1)));
  BOOST_CHECK((Z(2) >= 1));
  BOOST_CHECK((2 >= Z(1)));

  BOOST_CHECK((Z(2) >= Z(1)));
  BOOST_CHECK((!(Z(1) >= Z(2))));
  BOOST_CHECK((Z(1) >= Z(1)));

  BOOST_CHECK((Z::from_string("deadbeefbada550ff1ce", 16) >=
               Z::from_string("deadbeefbadf00d", 16)));

  // trailing_zeros
  BOOST_CHECK(Z(1).trailing_zeros() == 0);
  BOOST_CHECK(Z(2).trailing_zeros() == 1);
  BOOST_CHECK(Z(3).trailing_zeros() == 0);
  BOOST_CHECK(Z(4).trailing_zeros() == 2);
  BOOST_CHECK(Z(5).trailing_zeros() == 0);
  BOOST_CHECK(Z(-1).trailing_zeros() == 0);
  BOOST_CHECK(Z(-2).trailing_zeros() == 1);
  BOOST_CHECK(Z(-3).trailing_zeros() == 0);

  // trailing_ones
  BOOST_CHECK(Z(0).trailing_ones() == 0);
  BOOST_CHECK(Z(1).trailing_ones() == 1);
  BOOST_CHECK(Z(2).trailing_ones() == 0);
  BOOST_CHECK(Z(3).trailing_ones() == 2);
  BOOST_CHECK(Z(4).trailing_ones() == 0);
  BOOST_CHECK(Z(5).trailing_ones() == 1);
  BOOST_CHECK(Z(-2).trailing_ones() == 0);
  BOOST_CHECK(Z(-3).trailing_ones() == 1);

  // size_in_bits
  BOOST_CHECK(Z(0).size_in_bits() == 1);
  BOOST_CHECK(Z(1).size_in_bits() == 1);
  BOOST_CHECK(Z(2).size_in_bits() == 2);
  BOOST_CHECK(Z(3).size_in_bits() == 2);
  BOOST_CHECK(Z(4).size_in_bits() == 3);
  BOOST_CHECK(Z(5).size_in_bits() == 3);
  BOOST_CHECK(Z(6).size_in_bits() == 3);
  BOOST_CHECK(Z(7).size_in_bits() == 3);
  BOOST_CHECK(Z(8).size_in_bits() == 4);
  BOOST_CHECK(Z(9).size_in_bits() == 4);
  BOOST_CHECK(Z(-2).size_in_bits() == 2);
  BOOST_CHECK(Z(-3).size_in_bits() == 2);

  // next_power_of_2
  BOOST_CHECK(Z(0).next_power_of_2() == 1);
  BOOST_CHECK(Z(1).next_power_of_2() == 1);
  BOOST_CHECK(Z(2).next_power_of_2() == 2);
  BOOST_CHECK(Z(3).next_power_of_2() == 4);
  BOOST_CHECK(Z(4).next_power_of_2() == 4);
  BOOST_CHECK(Z(5).next_power_of_2() == 8);
  BOOST_CHECK(Z(6).next_power_of_2() == 8);
  BOOST_CHECK(Z(7).next_power_of_2() == 8);
  BOOST_CHECK(Z(8).next_power_of_2() == 8);
  BOOST_CHECK(Z(9).next_power_of_2() == 16);
  BOOST_CHECK(Z(10).next_power_of_2() == 16);
  BOOST_CHECK(Z(15).next_power_of_2() == 16);
  BOOST_CHECK(Z(16).next_power_of_2() == 16);
  BOOST_CHECK(Z(17).next_power_of_2() == 32);

  // min
  BOOST_CHECK((min(Z(1), Z(2)) == Z(1)));
  BOOST_CHECK((min(Z(2), Z(1)) == Z(1)));
  BOOST_CHECK((min(Z(2), Z(1), Z(3)) == Z(1)));
  BOOST_CHECK((min(Z(2), Z(1), Z(3), Z(-1)) == Z(-1)));

  // max
  BOOST_CHECK((max(Z(1), Z(2)) == Z(2)));
  BOOST_CHECK((max(Z(2), Z(1)) == Z(2)));
  BOOST_CHECK((max(Z(2), Z(1), Z(3)) == Z(3)));
  BOOST_CHECK((max(Z(2), Z(1), Z(3), Z(-1)) == Z(3)));

  // mod
  BOOST_CHECK((mod(Z(11), Z(3)) == Z(2)));
  BOOST_CHECK((mod(Z(11), Z(-3)) == Z(2)));
  BOOST_CHECK((mod(Z(-11), Z(3)) == Z(1)));
  BOOST_CHECK((mod(Z(-11), Z(-3)) == Z(1)));
  BOOST_CHECK((mod(Z(12), Z(3)) == Z(0)));
  BOOST_CHECK((mod(Z(12), Z(-3)) == Z(0)));
  BOOST_CHECK((mod(Z(-12), Z(3)) == Z(0)));
  BOOST_CHECK((mod(Z(-12), Z(-3)) == Z(0)));

  // abs
  BOOST_CHECK((abs(Z(0)) == Z(0)));
  BOOST_CHECK((abs(Z(2)) == Z(2)));
  BOOST_CHECK((abs(Z(-2)) == Z(2)));

  // gcd
  BOOST_CHECK((gcd(Z(0), Z(0)) == Z(0)));
  BOOST_CHECK((gcd(Z(0), Z(2)) == Z(2)));
  BOOST_CHECK((gcd(Z(2), Z(4)) == Z(2)));
  BOOST_CHECK((gcd(Z(-2), Z(4)) == Z(2)));
  BOOST_CHECK((gcd(Z(2), Z(-4)) == Z(2)));
  BOOST_CHECK((gcd(Z(-2), Z(-4)) == Z(2)));
  BOOST_CHECK((gcd(Z(3), Z(7)) == Z(1)));
  BOOST_CHECK((gcd(Z(21), Z(35)) == Z(7)));

  // lcm
  BOOST_CHECK((lcm(Z(0), Z(0)) == Z(0)));
  BOOST_CHECK((lcm(Z(0), Z(2)) == Z(0)));
  BOOST_CHECK((lcm(Z(2), Z(4)) == Z(4)));
  BOOST_CHECK((lcm(Z(-2), Z(4)) == Z(4)));
  BOOST_CHECK((lcm(Z(2), Z(-4)) == Z(4)));
  BOOST_CHECK((lcm(Z(-2), Z(-4)) == Z(4)));
  BOOST_CHECK((lcm(Z(3), Z(7)) == Z(21)));
  BOOST_CHECK((lcm(Z(21), Z(35)) == Z(105)));

  // gcd_extended
  Z g;
  Z a;
  Z b;
  Z u;
  Z v;
  gcd_extended(a = Z(0), b = Z(0), g, u, v);
  BOOST_CHECK((g == Z(0) && g == a * u + b * v));
  gcd_extended(a = Z(0), b = Z(2), g, u, v);
  BOOST_CHECK((g == Z(2) && g == a * u + b * v));
  gcd_extended(a = Z(2), b = Z(4), g, u, v);
  BOOST_CHECK((g == Z(2) && g == a * u + b * v));
  gcd_extended(a = Z(-2), b = Z(4), g, u, v);
  BOOST_CHECK((g == Z(2) && g == a * u + b * v));
  gcd_extended(a = Z(2), b = Z(-4), g, u, v);
  BOOST_CHECK((g == Z(2) && g == a * u + b * v));
  gcd_extended(a = Z(-2), b = Z(-4), g, u, v);
  BOOST_CHECK((g == Z(2) && g == a * u + b * v));
  gcd_extended(a = Z(3), b = Z(7), g, u, v);
  BOOST_CHECK((g == Z(1) && g == a * u + b * v));
  gcd_extended(a = Z(21), b = Z(35), g, u, v);
  BOOST_CHECK((g == Z(7) && g == a * u + b * v));
  gcd_extended(a = Z(3), b = Z(4), g, u, v);
  BOOST_CHECK((g == Z(1) && g == a * u + b * v));
  gcd_extended(a = Z(270), b = Z(192), g, u, v);
  BOOST_CHECK((g == Z(6) && g == a * u + b * v));

  // fits< T >
  BOOST_CHECK(Z(1).fits< int >());
  BOOST_CHECK(Z(-1).fits< int >());
  BOOST_CHECK(Z(std::numeric_limits< int >::max()).fits< int >());
  BOOST_CHECK(Z(std::numeric_limits< int >::min()).fits< int >());
  BOOST_CHECK(!(Z(std::numeric_limits< int >::max()) + 1).fits< int >());
  BOOST_CHECK(!(Z(std::numeric_limits< int >::min()) - 1).fits< int >());

  BOOST_CHECK(Z(0).fits< unsigned int >());
  BOOST_CHECK(Z(1).fits< unsigned int >());
  BOOST_CHECK(
      Z(std::numeric_limits< unsigned int >::max()).fits< unsigned int >());
  BOOST_CHECK(!Z(-1).fits< unsigned int >());
  BOOST_CHECK(!(Z(std::numeric_limits< unsigned int >::max()) + 1)
                   .fits< unsigned int >());

  BOOST_CHECK(Z(1).fits< long >());
  BOOST_CHECK(Z(-1).fits< long >());
  BOOST_CHECK(Z(std::numeric_limits< long >::max()).fits< long >());
  BOOST_CHECK(Z(std::numeric_limits< long >::min()).fits< long >());
  BOOST_CHECK(!(Z(std::numeric_limits< long >::max()) + 1).fits< long >());
  BOOST_CHECK(!(Z(std::numeric_limits< long >::min()) - 1).fits< long >());

  BOOST_CHECK(Z(0).fits< unsigned long >());
  BOOST_CHECK(Z(1).fits< unsigned long >());
  BOOST_CHECK(
      Z(std::numeric_limits< unsigned long >::max()).fits< unsigned long >());
  BOOST_CHECK(!Z(-1).fits< unsigned long >());
  BOOST_CHECK(!(Z(std::numeric_limits< unsigned long >::max()) + 1)
                   .fits< unsigned long >());

  BOOST_CHECK(Z(1).fits< long long >());
  BOOST_CHECK(Z(-1).fits< long long >());
  BOOST_CHECK(Z(std::numeric_limits< long long >::max()).fits< long long >());
  BOOST_CHECK(Z(std::numeric_limits< long long >::min()).fits< long long >());
  BOOST_CHECK(
      !(Z(std::numeric_limits< long long >::max()) + 1).fits< long long >());
  BOOST_CHECK(
      !(Z(std::numeric_limits< long long >::min()) - 1).fits< long long >());
  BOOST_CHECK(((Z(0x12345678) << 32) + Z(0x01234567)).fits< long long >());
  BOOST_CHECK(!((Z(0x82345678) << 32) + Z(0x01234567)).fits< long long >());
  BOOST_CHECK(!(-((Z(0x82345678) << 32) + Z(0x01234567))).fits< long long >());

  BOOST_CHECK(Z(0).fits< unsigned long long >());
  BOOST_CHECK(Z(1).fits< unsigned long long >());
  BOOST_CHECK(!Z(-1).fits< unsigned long long >());
  BOOST_CHECK(Z(std::numeric_limits< unsigned long long >::max())
                  .fits< unsigned long long >());
  BOOST_CHECK(!(Z(std::numeric_limits< unsigned long long >::max()) + 1)
                   .fits< unsigned long long >());
  BOOST_CHECK(
      ((Z(0x12345678) << 32) + Z(0x01234567)).fits< unsigned long long >());
  BOOST_CHECK(
      ((Z(0x82345678) << 32) + Z(0x01234567)).fits< unsigned long long >());
  BOOST_CHECK(
      !(-((Z(0x82345678) << 32) + Z(0x01234567))).fits< unsigned long long >());

  // to< T >
  BOOST_CHECK(Z(1).to< int >() == static_cast< int >(1));
  BOOST_CHECK(Z(-1).to< int >() == static_cast< int >(-1));
  BOOST_CHECK(Z(std::numeric_limits< int >::max()).to< int >() ==
              std::numeric_limits< int >::max());
  BOOST_CHECK(Z(std::numeric_limits< int >::min()).to< int >() ==
              std::numeric_limits< int >::min());

  BOOST_CHECK(Z(0).to< unsigned int >() == static_cast< unsigned int >(0));
  BOOST_CHECK(Z(1).to< unsigned int >() == static_cast< unsigned int >(1));
  BOOST_CHECK(
      Z(std::numeric_limits< unsigned int >::max()).to< unsigned int >() ==
      std::numeric_limits< unsigned int >::max());

  BOOST_CHECK(Z(1).to< long >() == static_cast< long >(1));
  BOOST_CHECK(Z(-1).to< long >() == static_cast< long >(-1));
  BOOST_CHECK(Z(std::numeric_limits< long >::max()).to< long >() ==
              std::numeric_limits< long >::max());
  BOOST_CHECK(Z(std::numeric_limits< long >::min()).to< long >() ==
              std::numeric_limits< long >::min());

  BOOST_CHECK(Z(0).to< unsigned long >() == static_cast< unsigned long >(0));
  BOOST_CHECK(Z(1).to< unsigned long >() == static_cast< unsigned long >(1));
  BOOST_CHECK(
      Z(std::numeric_limits< unsigned long >::max()).to< unsigned long >() ==
      std::numeric_limits< unsigned long >::max());

  BOOST_CHECK(Z(1).to< long long >() == static_cast< long long >(1));
  BOOST_CHECK(Z(-1).to< long long >() == static_cast< long long >(-1));
  BOOST_CHECK(Z(std::numeric_limits< long long >::max()).to< long long >() ==
              std::numeric_limits< long long >::max());
  BOOST_CHECK(Z(std::numeric_limits< long long >::min()).to< long long >() ==
              std::numeric_limits< long long >::min());
  BOOST_CHECK(((Z(0x12345678) << 32) + Z(0x01234567)).to< long long >() ==
              0x1234567801234567LL);

  BOOST_CHECK(Z(0).to< unsigned long long >() ==
              static_cast< unsigned long long >(0));
  BOOST_CHECK(Z(1).to< unsigned long long >() ==
              static_cast< unsigned long long >(1));
  BOOST_CHECK(Z(std::numeric_limits< unsigned long long >::max())
                  .to< unsigned long long >() ==
              std::numeric_limits< unsigned long long >::max());
  BOOST_CHECK(
      ((Z(0x12345678) << 32) + Z(0x01234567)).to< unsigned long long >() ==
      0x1234567801234567ULL);
  BOOST_CHECK(
      ((Z(0x82345678) << 32) + Z(0x01234567)).to< unsigned long long >() ==
      0x8234567801234567ULL);

  // operator<<
  boost::test_tools::output_test_stream output;
  output << Z(42);
  BOOST_CHECK(output.is_equal("42"));

  // ZNumber single_mask(const ZNumber& size);

  BOOST_CHECK(single_mask(Z(0)) == Z(0));
  BOOST_CHECK(single_mask((Z(1) << 32) - 1) == single_mask((Z(1) << 32) - 1));
  // On some platforms bigger values will work, but don't count on it.
  // BOOST_CHECK(single_mask(Z(1) << 34) == single_mask(Z(1) << 34));
  BOOST_CHECK(single_mask(Z(5)).str(2) == "11111");

  //  ZNumber double_mask(const ZNumber& low, const ZNumber& high);
  BOOST_CHECK(double_mask(Z(2), Z(5)).str(2) == "11100");
  BOOST_CHECK(double_mask(Z(0), Z(5)).str(2) == "11111");

  // ZNumber make_clipped_mask(
  //      const ZNumber& low, const ZNumber& size,
  //      const ZNumber& lower_clip, const ZNumber& size_clip);
  BOOST_CHECK(make_clipped_mask(Z(2), Z(5), Z(2), Z(5)).str(2) == "11111");
  BOOST_CHECK(make_clipped_mask(Z(0), Z(10), Z(2), Z(5)).str(2) == "11111");
  BOOST_CHECK(make_clipped_mask(Z(0), Z(7), Z(2), Z(5)).str(2) == "11111");
  BOOST_CHECK(make_clipped_mask(Z(0), Z(5), Z(2), Z(5)).str(2) == "111");
  BOOST_CHECK(make_clipped_mask(Z(0), Z(2), Z(2), Z(5)).str(2) == "0");
  BOOST_CHECK(make_clipped_mask(Z(0), Z(1), Z(2), Z(5)).str(2) == "0");
  BOOST_CHECK(make_clipped_mask(Z(3), Z(2), Z(2), Z(5)).str(2) == "110");
  BOOST_CHECK(make_clipped_mask(Z(2), Z(4), Z(2), Z(5)).str(2) == "1111");
  BOOST_CHECK(make_clipped_mask(Z(2), Z(5), Z(2), Z(5)).str(2) == "11111");
  BOOST_CHECK(make_clipped_mask(Z(6), Z(2), Z(2), Z(5)).str(2) == "10000");
  BOOST_CHECK(make_clipped_mask(Z(7), Z(5), Z(2), Z(5)).str(2) == "0");
}
