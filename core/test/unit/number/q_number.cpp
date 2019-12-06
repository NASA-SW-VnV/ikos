/*******************************************************************************
 *
 * Tests for QNumber
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

#define BOOST_TEST_MODULE test_q_number
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION >= 107100
#include <boost/test/tools/output_test_stream.hpp>
#else
#include <boost/test/output_test_stream.hpp>
#endif

#include <ikos/core/number/q_number.hpp>

BOOST_AUTO_TEST_CASE(test_q_number) {
  using Z = ikos::core::ZNumber;
  using Q = ikos::core::QNumber;
  using NumberError = ikos::core::NumberError;

  // constructors
  BOOST_CHECK((Q() == Q(0)));
  BOOST_CHECK((Q(1) == Q(static_cast< int >(1))));
  BOOST_CHECK((Q(-1) == Q(static_cast< int >(-1))));
  BOOST_CHECK((Q(1) == Q(static_cast< unsigned int >(1))));
  BOOST_CHECK((Q(1) == Q(static_cast< long >(1))));
  BOOST_CHECK((Q(-1) == Q(static_cast< long >(-1))));
  BOOST_CHECK((Q(1) == Q(static_cast< unsigned long >(1))));
  BOOST_CHECK((Q(1) == Q(static_cast< long long >(1))));
  BOOST_CHECK((Q(-1) == Q(static_cast< long long >(-1))));
  BOOST_CHECK((Q(1) == Q(static_cast< unsigned long long >(1))));

  BOOST_CHECK((Q(1, 2) == Q(static_cast< int >(2), 4)));
  BOOST_CHECK((Q(-1, 2) == Q(static_cast< int >(-2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(static_cast< unsigned int >(2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(static_cast< long >(2), 4)));
  BOOST_CHECK((Q(-1, 2) == Q(static_cast< long >(-2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(static_cast< unsigned long >(2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(static_cast< long long >(2), 4)));
  BOOST_CHECK((Q(-1, 2) == Q(static_cast< long long >(-2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(static_cast< unsigned long long >(2), 4)));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< int >(4))));
  BOOST_CHECK((Q(-1, 2) == Q(2, static_cast< int >(-4))));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< unsigned int >(4))));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< long >(4))));
  BOOST_CHECK((Q(-1, 2) == Q(2, static_cast< long >(-4))));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< unsigned long >(4))));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< long long >(4))));
  BOOST_CHECK((Q(-1, 2) == Q(2, static_cast< long long >(-4))));
  BOOST_CHECK((Q(1, 2) == Q(2, static_cast< unsigned long long >(4))));

  BOOST_CHECK((Q(1) == Q(Z(1))));
  BOOST_CHECK((Q(1, 2) == Q(Z(2), Z(4))));

  // operator=
  Q n;
  BOOST_CHECK((Q(1) == (n = static_cast< int >(1))));
  BOOST_CHECK((Q(-1) == (n = static_cast< int >(-1))));
  BOOST_CHECK((Q(1) == (n = static_cast< unsigned int >(1))));
  BOOST_CHECK((Q(1) == (n = static_cast< long >(1))));
  BOOST_CHECK((Q(-1) == (n = static_cast< long >(-1))));
  BOOST_CHECK((Q(1) == (n = static_cast< unsigned long >(1))));
  BOOST_CHECK((Q(1) == (n = static_cast< long long >(1))));
  BOOST_CHECK((Q(-1) == (n = static_cast< long long >(-1))));
  BOOST_CHECK((Q(1) == (n = static_cast< unsigned long long >(1))));
  BOOST_CHECK((Q(1) == (n = Z(1))));

  // from_string
  BOOST_CHECK((Q(1) == Q::from_string("1")));
  BOOST_CHECK((Q(-1) == Q::from_string("-1")));
  BOOST_CHECK((Q(1) == Q::from_string("01")));
  BOOST_CHECK((Q(1) == Q::from_string(" 1")));
  BOOST_CHECK((Q(1) == Q::from_string("1\n")));
  BOOST_CHECK((Q(101) == Q::from_string("1 0 1")));
  BOOST_CHECK((Q(1, 2) == Q::from_string("1/2")));
  BOOST_CHECK((Q(1, 2) == Q::from_string("2 / 4")));
  BOOST_CHECK((Q(1, 2) == Q::from_string("2 /\n4")));
  BOOST_CHECK((Q(1) == Q::from_string("1", 10)));
  BOOST_CHECK((Q(2) == Q::from_string("10", 2)));
  BOOST_CHECK((Q(3) == Q::from_string("11", 2)));
  BOOST_CHECK((Q(1, 2) == Q::from_string("10/100", 2)));
  BOOST_CHECK((Q(10) == Q::from_string("A", 16)));
  BOOST_CHECK((Q(10) == Q::from_string("a", 16)));
  BOOST_CHECK((Q(0xFF) == Q::from_string("FF", 16)));
  BOOST_CHECK((Q(1, 17) == Q::from_string("A/AA", 16)));
  BOOST_CHECK_THROW((Q::from_string("a")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("12a")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("a12")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("0.12")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("12\na")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("1a/2")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("1 / 2a")), NumberError);
  // BOOST_CHECK_THROW((Q::from_string("1/0")), NumberError);
  BOOST_CHECK_THROW((Q::from_string("NaN")), NumberError);

  // operator+
  BOOST_CHECK((Q(1) + Q(2) == Q(3)));
  BOOST_CHECK((Q(1) + 2 == Q(3)));
  BOOST_CHECK((1 + Q(2) == Q(3)));
  n = 1;
  BOOST_CHECK((Q(3) == (n += Q(2))));
  n = 1;
  BOOST_CHECK((Q(3) == (n += 2)));

  BOOST_CHECK((Q(1) + Q(-2) == Q(-1)));
  BOOST_CHECK((Q(-1) + Q(2) == Q(1)));
  BOOST_CHECK((Q(-1) + Q(-2) == Q(-3)));

  BOOST_CHECK((Q(1, 2) + Q(1, 3) == Q(5, 6)));
  BOOST_CHECK((Q(-1, 2) + Q(1, 3) == Q(-1, 6)));
  BOOST_CHECK((Q(1, 2) + Q(-1, 3) == Q(1, 6)));
  BOOST_CHECK((Q(-1, 2) + Q(-1, 3) == Q(-5, 6)));

  BOOST_CHECK((Q::from_string("deadbeefbada550ff1ce", 16) +
                   Q::from_string("deadbeefbadf00d", 16) ==
               Q::from_string("deadccda96c950bde1db", 16)));

  // operator-
  BOOST_CHECK((Q(3) - Q(2) == Q(1)));
  BOOST_CHECK((Q(3) - 2 == Q(1)));
  BOOST_CHECK((3 - Q(2) == Q(1)));
  n = 3;
  BOOST_CHECK((Q(1) == (n -= Q(2))));
  n = 3;
  BOOST_CHECK((Q(1) == (n -= 2)));

  BOOST_CHECK((Q(-3) - Q(1) == Q(-4)));
  BOOST_CHECK((Q(3) - Q(-1) == Q(4)));
  BOOST_CHECK((Q(-2) - Q(-3) == Q(1)));

  BOOST_CHECK((Q(1, 2) - Q(1, 3) == Q(1, 6)));
  BOOST_CHECK((Q(-1, 2) - Q(1, 3) == Q(-5, 6)));
  BOOST_CHECK((Q(1, 2) - Q(-1, 3) == Q(5, 6)));
  BOOST_CHECK((Q(-1, 2) - Q(-1, 3) == Q(-1, 6)));

  BOOST_CHECK((Q::from_string("deadbeefbada550ff1ce", 16) -
                   Q::from_string("deadbeefbadf00d", 16) ==
               Q::from_string("deadb104deeb596201c1", 16)));

  // operator*
  BOOST_CHECK((Q(3) * Q(2) == Q(6)));
  BOOST_CHECK((Q(3) * 2 == Q(6)));
  BOOST_CHECK((3 * Q(2) == Q(6)));
  n = 3;
  BOOST_CHECK((Q(6) == (n *= Q(2))));
  n = 3;
  BOOST_CHECK((Q(6) == (n *= 2)));

  BOOST_CHECK((Q(3) * Q(-2) == Q(-6)));
  BOOST_CHECK((Q(-3) * Q(2) == Q(-6)));
  BOOST_CHECK((Q(-3) * Q(-2) == Q(6)));

  BOOST_CHECK((Q(1, 2) * Q(1, 3) == Q(1, 6)));
  BOOST_CHECK((Q(-1, 2) * Q(1, 3) == Q(-1, 6)));
  BOOST_CHECK((Q(1, 2) * Q(-1, 3) == Q(-1, 6)));
  BOOST_CHECK((Q(-1, 2) * Q(-1, 3) == Q(1, 6)));

  BOOST_CHECK((Q::from_string("deadbeefbada550ff1ce", 16) *
                   Q::from_string("deadbeefbadf00d", 16) ==
               Q::from_string("c1b1cd13668200953a2fd455234a6b66776", 16)));

  // operator/
  BOOST_CHECK((Q(3) / Q(2) == Q(3, 2)));
  BOOST_CHECK((Q(3) / 2 == Q(3, 2)));
  BOOST_CHECK((3 / Q(2) == Q(3, 2)));
  n = 3;
  BOOST_CHECK((Q(3, 2) == (n /= Q(2))));
  n = 3;
  BOOST_CHECK((Q(3, 2) == (n /= 2)));

  BOOST_CHECK((Q(11) / Q(3) == Q(11, 3)));
  BOOST_CHECK((Q(11) / Q(-3) == Q(-11, 3)));
  BOOST_CHECK((Q(-11) / Q(3) == Q(-11, 3)));
  BOOST_CHECK((Q(-11) / Q(-3) == Q(11, 3)));

  BOOST_CHECK((Q(12) / Q(3) == Q(12, 3)));
  BOOST_CHECK((Q(12) / Q(-3) == Q(-12, 3)));
  BOOST_CHECK((Q(-12) / Q(3) == Q(-12, 3)));
  BOOST_CHECK((Q(-12) / Q(-3) == Q(12, 3)));

  // operator++
  n = 0;
  BOOST_CHECK((n++ == Q(0)));
  n = 0;
  BOOST_CHECK((++n == Q(1)));

  // operator--
  n = 10;
  BOOST_CHECK((n-- == Q(10)));
  n = 10;
  BOOST_CHECK((--n == Q(9)));

  // operator==
  BOOST_CHECK((Q(1) == Q(1)));
  BOOST_CHECK((Q(1) == 1));
  BOOST_CHECK((1 == Q(1)));

  BOOST_CHECK((Q(1, 2) == Q(2, 4)));
  BOOST_CHECK((!(Q(1, 2) == Q(2, 1))));

  // operator!=
  BOOST_CHECK((Q(1) != Q(2)));
  BOOST_CHECK((Q(1) != 2));
  BOOST_CHECK((1 != Q(2)));

  BOOST_CHECK((Q(1, 2) != Q(2, 1)));
  BOOST_CHECK((!(Q(1, 2) != Q(2, 4))));

  // operator<
  BOOST_CHECK((Q(1) < Q(2)));
  BOOST_CHECK((Q(1) < 2));
  BOOST_CHECK((1 < Q(2)));

  BOOST_CHECK((Q(1, 3) < Q(1, 2)));
  BOOST_CHECK((!(Q(1, 2) < Q(1, 3))));
  BOOST_CHECK((!(Q(1, 2) < Q(1, 2))));

  BOOST_CHECK((Q::from_string("deadbeefbadf00d", 16) <
               Q::from_string("deadbeefbada550ff1ce", 16)));

  // operator<=
  BOOST_CHECK((Q(1) <= Q(2)));
  BOOST_CHECK((Q(1) <= 2));
  BOOST_CHECK((1 <= Q(2)));

  BOOST_CHECK((Q(1, 3) <= Q(1, 2)));
  BOOST_CHECK((!(Q(1, 2) <= Q(1, 3))));
  BOOST_CHECK((Q(1, 2) <= Q(1, 2)));

  BOOST_CHECK((Q::from_string("deadbeefbadf00d", 16) <=
               Q::from_string("deadbeefbada550ff1ce", 16)));

  // operator>
  BOOST_CHECK((Q(2) > Q(1)));
  BOOST_CHECK((Q(2) > 1));
  BOOST_CHECK((2 > Q(1)));

  BOOST_CHECK((Q(1, 2) > Q(1, 3)));
  BOOST_CHECK((!(Q(1, 3) > Q(1, 2))));
  BOOST_CHECK((!(Q(1, 2) > Q(1, 2))));

  BOOST_CHECK((Q::from_string("deadbeefbada550ff1ce", 16) >
               Q::from_string("deadbeefbadf00d", 16)));

  // operator>=
  BOOST_CHECK((Q(2) >= Q(1)));
  BOOST_CHECK((Q(2) >= 1));
  BOOST_CHECK((2 >= Q(1)));

  BOOST_CHECK((Q(1, 2) >= Q(1, 3)));
  BOOST_CHECK((!(Q(1, 3) >= Q(1, 2))));
  BOOST_CHECK((Q(1, 2) >= Q(1, 2)));

  BOOST_CHECK((Q::from_string("deadbeefbada550ff1ce", 16) >=
               Q::from_string("deadbeefbadf00d", 16)));

  // numerator
  BOOST_CHECK((Q(1, 2).numerator() == Z(1)));
  BOOST_CHECK((Q(1, -2).numerator() == Z(-1)));
  BOOST_CHECK((Q(-1, 2).numerator() == Z(-1)));
  BOOST_CHECK((Q(-1, -2).numerator() == Z(1)));
  BOOST_CHECK((Q(2, 4).numerator() == Z(1)));
  BOOST_CHECK((Q(7, 11).numerator() == Z(7)));

  // denominator
  BOOST_CHECK((Q(1, 2).denominator() == Z(2)));
  BOOST_CHECK((Q(1, -2).denominator() == Z(2)));
  BOOST_CHECK((Q(-1, 2).denominator() == Z(2)));
  BOOST_CHECK((Q(-1, -2).denominator() == Z(2)));
  BOOST_CHECK((Q(2, 4).denominator() == Z(2)));
  BOOST_CHECK((Q(7, 11).denominator() == Z(11)));

  // round_to_upper
  BOOST_CHECK((Q(1, 3).round_to_upper() == Z(1)));
  BOOST_CHECK((Q(4, 3).round_to_upper() == Z(2)));
  BOOST_CHECK((Q(1).round_to_upper() == Z(1)));
  BOOST_CHECK((Q(-1, 3).round_to_upper() == Z(0)));
  BOOST_CHECK((Q(-4, 3).round_to_upper() == Z(-1)));

  // round_to_lower
  BOOST_CHECK((Q(1, 3).round_to_lower() == Z(0)));
  BOOST_CHECK((Q(4, 3).round_to_lower() == Z(1)));
  BOOST_CHECK((Q(1).round_to_lower() == Z(1)));
  BOOST_CHECK((Q(-1, 3).round_to_lower() == Z(-1)));
  BOOST_CHECK((Q(-4, 3).round_to_lower() == Z(-2)));

  // min
  BOOST_CHECK((min(Q(1), Q(2)) == Q(1)));
  BOOST_CHECK((min(Q(2), Q(1)) == Q(1)));
  BOOST_CHECK((min(Q(2), Q(1), Q(3)) == Q(1)));
  BOOST_CHECK((min(Q(2), Q(1), Q(3), Q(-1)) == Q(-1)));

  // max
  BOOST_CHECK((max(Q(1), Q(2)) == Q(2)));
  BOOST_CHECK((max(Q(2), Q(1)) == Q(2)));
  BOOST_CHECK((max(Q(2), Q(1), Q(3)) == Q(3)));
  BOOST_CHECK((max(Q(2), Q(1), Q(3), Q(-1)) == Q(3)));

  // abs
  BOOST_CHECK((abs(Q(0)) == Q(0)));
  BOOST_CHECK((abs(Q(2)) == Q(2)));
  BOOST_CHECK((abs(Q(-2)) == Q(2)));
  BOOST_CHECK((abs(Q(1, 2)) == Q(1, 2)));
  BOOST_CHECK((abs(Q(-1, 2)) == Q(1, 2)));
  BOOST_CHECK((abs(Q(1, -2)) == Q(1, 2)));
  BOOST_CHECK((abs(Q(-1, -2)) == Q(1, 2)));

  // operator<<
  boost::test_tools::output_test_stream output;
  output << Q(1, 2);
  BOOST_CHECK(output.is_equal("1/2"));
}
