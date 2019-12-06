/*******************************************************************************
 *
 * Tests for machine_int::Congruence
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

#define BOOST_TEST_MODULE test_machine_integer_congruence
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/machine_int/congruence.hpp>

using Z = ikos::core::ZNumber;
using Int = ikos::core::MachineInt;
using Congruence = ikos::core::machine_int::Congruence;
using ikos::core::Signed;
using ikos::core::Unsigned;

BOOST_AUTO_TEST_CASE(test_constructors) {
  // Congruence(Int)
  BOOST_CHECK(Congruence(Int(0, 8, Signed)) ==
              Congruence(Int(0, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed)) ==
              Congruence(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(-128, 8, Signed)) ==
              Congruence(Int(0, 8, Signed), Int(-128, 8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed)) ==
              Congruence(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned), Int(255, 8, Unsigned)));

  // Congruence(Int a, Int b)
  BOOST_CHECK(Congruence(Int(0, 8, Signed), Int(0, 8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(0, 8, Signed), Int(1, 8, Signed)) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed), Int(0, 8, Signed)) ==
              Congruence(Int(1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed), Int(1, 8, Signed)) ==
              Congruence(Int(1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed), Int(-1, 8, Signed)) ==
              Congruence(Int(1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(2, 8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(-2, 8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(1, 8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed), Int(1, 8, Signed)) ==
              Congruence(Int(127, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed), Int(-1, 8, Signed)) ==
              Congruence(Int(127, 8, Signed), Int(126, 8, Signed)));

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned), Int(0, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)) ==
              Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned), Int(1, 8, Unsigned)) ==
              Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned), Int(255, 8, Unsigned)) ==
              Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(2, 8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(4, 8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned), Int(1, 8, Unsigned)) ==
              Congruence(Int(255, 8, Unsigned), Int(1, 8, Unsigned)));

  // tricky cases
  BOOST_CHECK(Congruence(Int(254, 8, Unsigned), Int(2, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(250, 8, Unsigned), Int(6, 8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned), Int(6, 8, Unsigned)));

  // Congruence(Z a, Z b)
  BOOST_CHECK(Congruence(Z(128), Z(-1), 8, Signed) ==
              Congruence(Z(128), Z(127), 8, Signed));
  BOOST_CHECK(Congruence(Z(254), Z(2), 8, Signed) ==
              Congruence(Z(0), Z(2), 8, Signed));
  BOOST_CHECK(Congruence(Z(254), Z(128), 8, Signed) ==
              Congruence(Z(0), Z(-126), 8, Signed));
  BOOST_CHECK(Congruence(Z(254), Z(2), 8, Unsigned) ==
              Congruence(Z(0), Z(2), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(250), Z(6), 8, Unsigned) ==
              Congruence(Z(0), Z(6), 8, Unsigned));

  // Congruence::top()/bottom()
  BOOST_CHECK(Congruence::top(1, Signed) == Congruence(Z(1), Z(0), 1, Signed));
  BOOST_CHECK(Congruence::top(8, Signed) == Congruence(Z(1), Z(0), 8, Signed));

  BOOST_CHECK(Congruence::top(1, Unsigned) ==
              Congruence(Z(1), Z(0), 1, Unsigned));
  BOOST_CHECK(Congruence::top(8, Unsigned) ==
              Congruence(Z(1), Z(0), 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_bit_width) {
  BOOST_CHECK(Congruence(Int(0, 4, Signed)).bit_width() == 4);
  BOOST_CHECK(Congruence(Int(0, 4, Unsigned)).bit_width() == 4);
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).bit_width() == 8);
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)).bit_width() == 8);
}

BOOST_AUTO_TEST_CASE(test_sign) {
  BOOST_CHECK(Congruence(Int(0, 4, Signed)).sign() == Signed);
  BOOST_CHECK(Congruence(Int(0, 4, Unsigned)).sign() == Unsigned);
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).sign() == Signed);
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)).sign() == Unsigned);
}

BOOST_AUTO_TEST_CASE(test_modulus_and_residue) {
  BOOST_CHECK(Congruence(Int(0, 8, Signed), Int(1, 8, Signed)).modulus() ==
              Z(0));
  BOOST_CHECK(Congruence(Int(0, 8, Signed), Int(1, 8, Signed)).residue() ==
              Z(1));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).modulus() ==
              Z(0));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).residue() ==
              Z(1));
}

BOOST_AUTO_TEST_CASE(test_bottom) {
  BOOST_CHECK(Congruence::bottom(8, Signed).is_bottom());
  BOOST_CHECK(!Congruence::top(8, Signed).is_bottom());
  BOOST_CHECK(!Congruence(Int(0, 8, Signed), Int(1, 8, Signed)).is_bottom());
  BOOST_CHECK(!Congruence(Int(1, 8, Signed), Int(0, 8, Signed)).is_bottom());

  BOOST_CHECK(Congruence::bottom(8, Unsigned).is_bottom());
  BOOST_CHECK(!Congruence::top(8, Unsigned).is_bottom());
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).is_bottom());
  BOOST_CHECK(
      !Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).is_bottom());
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).is_bottom());
}

BOOST_AUTO_TEST_CASE(test_top) {
  BOOST_CHECK(!Congruence::bottom(8, Signed).is_top());
  BOOST_CHECK(Congruence::top(8, Signed).is_top());
  BOOST_CHECK(!Congruence(Int(0, 8, Signed), Int(1, 8, Signed)).is_top());
  BOOST_CHECK(Congruence(Int(1, 8, Signed), Int(0, 8, Signed)).is_top());

  BOOST_CHECK(!Congruence::bottom(8, Unsigned).is_top());
  BOOST_CHECK(Congruence::top(8, Unsigned).is_top());
  BOOST_CHECK(!Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).is_top());
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).is_top());
  BOOST_CHECK(!Congruence(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_bottom) {
  Congruence i(Int(0, 8, Signed));
  i.set_to_bottom();
  BOOST_CHECK(i.is_bottom());
  BOOST_CHECK(!i.is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_top) {
  Congruence i(Int(0, 8, Signed));
  i.set_to_top();
  BOOST_CHECK(!i.is_bottom());
  BOOST_CHECK(i.is_top());
}

BOOST_AUTO_TEST_CASE(test_leq) {
  BOOST_CHECK(Congruence::bottom(8, Signed).leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence::bottom(8, Signed).leq(Congruence::top(8, Signed)));

  BOOST_CHECK(!Congruence::top(8, Signed).leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence::top(8, Signed).leq(Congruence::top(8, Signed)));

  // Signed

  BOOST_CHECK(
      !Congruence(Int(0, 8, Signed)).leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).leq(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(0, 8, Signed))
                  .leq(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(0, 8, Signed))
                   .leq(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(
      !Congruence(Int(127, 8, Signed)).leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed)).leq(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed))
                  .leq(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed))
                   .leq(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .leq(Congruence::top(8, Signed)));
  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .leq(Congruence(Int(4, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .leq(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .leq(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .leq(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .leq(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                  .leq(Congruence::top(8, Signed)));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .leq(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .leq(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(
      !Congruence(Z(128), Z(7), 8, Signed).leq(Congruence::bottom(8, Signed)));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).leq(Congruence::top(8, Signed)));
  BOOST_CHECK(!Congruence(Z(128), Z(7), 8, Signed)
                   .leq(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .leq(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  // Unsigned

  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned)).leq(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).leq(Congruence::top(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned))
                  .leq(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(!Congruence(Int(0, 8, Unsigned))
                   .leq(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));

  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned)).leq(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(255, 8, Unsigned)).leq(Congruence::top(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned))
                  .leq(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned))
                   .leq(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                   .leq(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .leq(Congruence::top(8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                   .leq(Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                   .leq(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                   .leq(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .leq(Congruence::top(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .leq(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(!Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                   .leq(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                   .leq(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                  .leq(Congruence::top(8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                   .leq(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                  .leq(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_equals) {
  BOOST_CHECK(
      Congruence::bottom(8, Signed).equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(
      !Congruence::bottom(8, Signed).equals(Congruence::top(8, Signed)));

  BOOST_CHECK(
      !Congruence::top(8, Signed).equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(Congruence::top(8, Signed).equals(Congruence::top(8, Signed)));

  // Signed

  BOOST_CHECK(
      !Congruence(Int(0, 8, Signed)).equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Signed)).equals(Congruence::top(8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).equals(Congruence(Int(0, 8, Signed))));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Signed)).equals(Congruence(Int(1, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(0, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(0, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(
      !Congruence(Int(127, 8, Signed)).equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(
      !Congruence(Int(127, 8, Signed)).equals(Congruence::top(8, Signed)));
  BOOST_CHECK(
      Congruence(Int(127, 8, Signed)).equals(Congruence(Int(127, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .equals(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .equals(Congruence(Int(2, 8, Signed), Int(2, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .equals(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .equals(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .equals(Congruence(Int(4, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .equals(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                  .equals(Congruence(Int(127, 8, Signed), Int(7, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Int(127, 8, Signed), Int(7, 8, Signed))
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  BOOST_CHECK(!Congruence(Z(128), Z(7), 8, Signed)
                   .equals(Congruence::bottom(8, Signed)));
  BOOST_CHECK(
      !Congruence(Z(128), Z(7), 8, Signed).equals(Congruence::top(8, Signed)));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .equals(Congruence(Z(128), Z(7), 8, Signed)));
  BOOST_CHECK(!Congruence(Z(128), Z(7), 8, Signed)
                   .equals(Congruence(Z(128), Z(6), 8, Signed)));
  BOOST_CHECK(!Congruence(Z(128), Z(7), 8, Signed)
                   .equals(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Congruence(Z(128), Z(7), 8, Signed)
                   .equals(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))));

  // Unsigned

  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned)).equals(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned)).equals(Congruence::top(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).equals(Congruence(Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned)).equals(Congruence(Int(1, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(0, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned))
                   .equals(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned)).equals(Congruence::top(8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned))
                  .equals(Congruence(Int(255, 8, Unsigned))));
  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned))
                   .equals(Congruence(Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                   .equals(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                   .equals(Congruence::top(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .equals(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .equals(Congruence(Int(2, 8, Unsigned), Int(2, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
           .equals(Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
           .equals(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                   .equals(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                   .equals(Congruence::top(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
          .equals(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
           .equals(Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));

  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                   .equals(Congruence::bottom(8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
                   .equals(Congruence::top(8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
          .equals(Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
           .equals(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))));

  // tricky cases
  BOOST_CHECK(Congruence(Z(254), Z(2), 8, Signed)
                  .equals(Congruence(Z(0), Z(2), 8, Signed)));
  BOOST_CHECK(Congruence(Z(254), Z(128), 8, Signed)
                  .equals(Congruence(Z(0), Z(-126), 8, Signed)));
  BOOST_CHECK(Congruence(Z(254), Z(2), 8, Unsigned)
                  .equals(Congruence(Z(0), Z(2), 8, Unsigned)));
  BOOST_CHECK(Congruence(Z(250), Z(6), 8, Unsigned)
                  .equals(Congruence(Z(0), Z(6), 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(255, 8, Unsigned), Int(7, 8, Unsigned))
          .equals(Congruence(Int(0, 8, Unsigned), Int(7, 8, Unsigned))));
  BOOST_CHECK(
      Congruence(Int(254, 8, Unsigned), Int(7, 8, Unsigned))
          .equals(Congruence(Int(0, 8, Unsigned), Int(7, 8, Unsigned))));
  BOOST_CHECK(
      Congruence(Int(253, 8, Unsigned), Int(7, 8, Unsigned))
          .equals(Congruence(Int(0, 8, Unsigned), Int(7, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_join) {
  BOOST_CHECK(Congruence::top(8, Signed).join(Congruence::bottom(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence::top(8, Signed).join(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      Congruence::bottom(8, Signed).join(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::bottom(8, Signed).join(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  // Signed

  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).join(Congruence::bottom(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).join(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).join(Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).join(Congruence(Int(1, 8, Signed))) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).join(Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(-128, 8, Signed)).join(Congruence(Int(127, 8, Signed))) ==
      Congruence(Z(255), Z(127), 8, Signed));
  BOOST_CHECK(
      Congruence(Int(-127, 8, Signed)).join(Congruence(Int(127, 8, Signed))) ==
      Congruence(Z(254), Z(127), 8, Signed));

  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .join(Congruence::bottom(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .join(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .join(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .join(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .join(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .join(Congruence::bottom(8, Signed)) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .join(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .join(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .join(Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .join(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).join(Congruence::bottom(8, Signed)) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).join(Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).join(Congruence(Int(0, 8, Signed))) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .join(Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .join(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).join(Congruence::bottom(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).join(Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).join(Congruence(Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned)).join(Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(255, 8, Unsigned)).join(Congruence(Int(0, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .join(Congruence::bottom(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .join(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .join(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .join(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .join(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .join(Congruence::bottom(8, Unsigned)) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .join(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .join(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .join(Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .join(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .join(Congruence::bottom(8, Unsigned)) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .join(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .join(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .join(Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned)));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .join(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_widening) {
  BOOST_CHECK(Congruence::top(8, Signed).widening(
                  Congruence::bottom(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(Congruence::top(8, Signed).widening(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      Congruence::bottom(8, Signed).widening(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::bottom(8, Signed).widening(
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));

  // Signed

  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).widening(Congruence::bottom(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).widening(Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).widening(Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).widening(Congruence(Int(1, 8, Signed))) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).widening(Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(-128, 8, Signed))
                  .widening(Congruence(Int(127, 8, Signed))) ==
              Congruence(Z(255), Z(127), 8, Signed));
  BOOST_CHECK(Congruence(Int(-127, 8, Signed))
                  .widening(Congruence(Int(127, 8, Signed))) ==
              Congruence(Z(254), Z(127), 8, Signed));

  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .widening(Congruence::bottom(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .widening(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .widening(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .widening(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .widening(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .widening(Congruence::bottom(8, Signed)) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .widening(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .widening(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .widening(Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .widening(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .widening(Congruence::bottom(8, Signed)) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .widening(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .widening(Congruence(Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .widening(Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .widening(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  // Unsigned

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned))
                  .widening(Congruence::bottom(8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).widening(Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned))
                  .widening(Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned))
                  .widening(Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned))
                  .widening(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .widening(Congruence::bottom(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .widening(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .widening(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .widening(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .widening(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .widening(Congruence::bottom(8, Unsigned)) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .widening(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .widening(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
          .widening(Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
          .widening(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .widening(Congruence::bottom(8, Unsigned)) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .widening(Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .widening(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Unsigned)
          .widening(Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Unsigned)
          .widening(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_meet) {
  BOOST_CHECK(Congruence::top(8, Signed).meet(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::top(8, Signed).meet(Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      Congruence::bottom(8, Signed).meet(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::bottom(8, Signed).meet(Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).meet(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).meet(Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).meet(Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).meet(Congruence(Int(1, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).meet(Congruence(Int(2, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(-128, 8, Signed)).meet(Congruence(Int(127, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(-127, 8, Signed)).meet(Congruence(Int(127, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .meet(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .meet(Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .meet(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .meet(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .meet(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .meet(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .meet(Congruence::top(8, Signed)) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .meet(Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .meet(Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .meet(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).meet(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).meet(Congruence::top(8, Signed)) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed).meet(Congruence(Int(0, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .meet(Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .meet(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  // Unsigned

  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).meet(Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).meet(Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).meet(Congruence(Int(1, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned)).meet(Congruence(Int(4, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(255, 8, Unsigned)).meet(Congruence(Int(0, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .meet(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .meet(Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .meet(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .meet(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .meet(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .meet(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .meet(Congruence::top(8, Unsigned)) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .meet(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .meet(Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .meet(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .meet(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .meet(Congruence::top(8, Unsigned)) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .meet(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .meet(Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .meet(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_narrowing) {
  BOOST_CHECK(
      Congruence::top(8, Signed).narrowing(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::top(8, Signed).narrowing(
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));

  BOOST_CHECK(
      Congruence::bottom(8, Signed).narrowing(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence::bottom(8, Signed).narrowing(
                  Congruence::top(8, Signed)) == Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).narrowing(Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).narrowing(Congruence::top(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).narrowing(Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).narrowing(Congruence(Int(1, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(0, 8, Signed)).narrowing(Congruence(Int(2, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(-128, 8, Signed))
                  .narrowing(Congruence(Int(127, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(-127, 8, Signed))
                  .narrowing(Congruence(Int(127, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
          .narrowing(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
      Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
          .narrowing(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .narrowing(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .narrowing(Congruence::top(8, Signed)) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
                  .narrowing(Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
          .narrowing(Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
      Congruence(Int(8, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(4, 8, Signed), Int(2, 8, Signed))
          .narrowing(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .narrowing(Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .narrowing(Congruence::top(8, Signed)) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Signed)
                  .narrowing(Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed)
          .narrowing(Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Signed)
          .narrowing(Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  // Unsigned

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned))
                  .narrowing(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(0, 8, Unsigned)).narrowing(Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned))
                  .narrowing(Congruence(Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned))
                  .narrowing(Congruence(Int(4, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned))
                  .narrowing(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .narrowing(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .narrowing(Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .narrowing(Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .narrowing(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
          .narrowing(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .narrowing(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .narrowing(Congruence::top(8, Unsigned)) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
                  .narrowing(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
          .narrowing(Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))
          .narrowing(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));

  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .narrowing(Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .narrowing(Congruence::top(8, Unsigned)) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(Congruence(Z(128), Z(7), 8, Unsigned)
                  .narrowing(Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Unsigned)
          .narrowing(Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(
      Congruence(Z(128), Z(7), 8, Unsigned)
          .narrowing(Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_singleton) {
  BOOST_CHECK((Congruence::bottom(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Congruence::top(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Congruence(Int(2, 8, Signed), Int(1, 8, Signed)).singleton() ==
               boost::none));
  BOOST_CHECK((Congruence(Int(0, 8, Signed)).singleton() ==
               boost::optional< Int >(Int(0, 8, Signed))));

  BOOST_CHECK((Congruence::bottom(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK((Congruence::top(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK(
      (Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)).singleton() ==
       boost::none));
  BOOST_CHECK((Congruence(Int(0, 8, Unsigned)).singleton() ==
               boost::optional< Int >(Int(0, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_contains) {
  BOOST_CHECK(!Congruence::bottom(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence::top(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).contains(Int(0, 8, Signed)));
  BOOST_CHECK(!Congruence(Int(0, 8, Signed)).contains(Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))
                  .contains(Int(0, 8, Signed)));
  BOOST_CHECK(!Congruence(Int(2, 8, Signed), Int(1, 8, Signed))
                   .contains(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(1, 8, Signed))
                  .contains(Int(-1, 8, Signed)));

  BOOST_CHECK(!Congruence::bottom(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence::top(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(0, 8, Unsigned)).contains(Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))
                  .contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(!Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))
                   .contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))
                  .contains(Int(3, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_trunc) {
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).trunc(6) ==
              Congruence(Int(0, 6, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed)).trunc(6) ==
              Congruence(Int(1, 6, Signed)));
  BOOST_CHECK(Congruence(Int(-128, 8, Signed)).trunc(6) ==
              Congruence(Int(0, 6, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed)).trunc(6) ==
              Congruence(Int(-1, 6, Signed)));

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)).trunc(6) ==
              Congruence(Int(0, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned)).trunc(6) ==
              Congruence(Int(1, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned)).trunc(6) ==
              Congruence(Int(63, 6, Unsigned)));

  BOOST_CHECK(Congruence(Int(1, 8, Signed), Int(0, 8, Signed)).trunc(6) ==
              Congruence(Int(1, 6, Signed), Int(0, 6, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)).trunc(6) ==
              Congruence(Int(2, 6, Signed), Int(0, 6, Signed)));
  BOOST_CHECK(Congruence(Int(2, 8, Signed), Int(1, 8, Signed)).trunc(6) ==
              Congruence(Int(2, 6, Signed), Int(1, 6, Signed)));
  BOOST_CHECK(Congruence(Int(8, 8, Signed), Int(3, 8, Signed)).trunc(6) ==
              Congruence(Int(8, 6, Signed), Int(3, 6, Signed)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed), Int(1, 8, Signed)).trunc(6) ==
              Congruence::top(6, Signed));

  BOOST_CHECK(Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).trunc(6) ==
              Congruence(Int(1, 6, Unsigned), Int(0, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)).trunc(6) ==
              Congruence(Int(2, 6, Unsigned), Int(0, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)).trunc(6) ==
              Congruence(Int(2, 6, Unsigned), Int(1, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(8, 8, Unsigned), Int(3, 8, Unsigned)).trunc(6) ==
              Congruence(Int(8, 6, Unsigned), Int(3, 6, Unsigned)));
  BOOST_CHECK(Congruence(Int(254, 8, Unsigned), Int(1, 8, Unsigned)).trunc(6) ==
              Congruence(Int(2, 6, Unsigned), Int(1, 6, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ext) {
  BOOST_CHECK(Congruence(Int(0, 6, Signed)).ext(8) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 6, Signed)).ext(8) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(-32, 6, Signed)).ext(8) ==
              Congruence(Int(-32, 8, Signed)));
  BOOST_CHECK(Congruence(Int(31, 6, Signed)).ext(8) ==
              Congruence(Int(31, 8, Signed)));

  BOOST_CHECK(Congruence(Int(0, 6, Unsigned)).ext(8) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 6, Unsigned)).ext(8) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(63, 6, Unsigned)).ext(8) ==
              Congruence(Int(63, 8, Unsigned)));

  BOOST_CHECK(Congruence(Int(1, 6, Signed), Int(0, 6, Signed)).ext(8) ==
              Congruence(Int(1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 6, Signed), Int(0, 6, Signed)).ext(8) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(2, 6, Signed), Int(1, 6, Signed)).ext(8) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(8, 6, Signed), Int(3, 6, Signed)).ext(8) ==
              Congruence(Int(8, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(Congruence(Int(31, 6, Signed), Int(1, 6, Signed)).ext(8) ==
              Congruence(Int(31, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(Congruence(Int(1, 6, Unsigned), Int(0, 6, Unsigned)).ext(8) ==
              Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 6, Unsigned), Int(0, 6, Unsigned)).ext(8) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(2, 6, Unsigned), Int(1, 6, Unsigned)).ext(8) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(8, 6, Unsigned), Int(3, 6, Unsigned)).ext(8) ==
              Congruence(Int(8, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(62, 6, Unsigned), Int(1, 6, Unsigned)).ext(8) ==
              Congruence(Int(62, 8, Unsigned), Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sign_cast) {
  BOOST_CHECK(Congruence(Int(0, 8, Signed)).sign_cast(Unsigned) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(1, 8, Signed)).sign_cast(Unsigned) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(-128, 8, Signed)).sign_cast(Unsigned) ==
              Congruence(Int(128, 8, Unsigned)));
  BOOST_CHECK(Congruence(Int(127, 8, Signed)).sign_cast(Unsigned) ==
              Congruence(Int(127, 8, Unsigned)));

  BOOST_CHECK(Congruence(Int(0, 8, Unsigned)).sign_cast(Signed) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(Congruence(Int(1, 8, Unsigned)).sign_cast(Signed) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(Congruence(Int(255, 8, Unsigned)).sign_cast(Signed) ==
              Congruence(Int(-1, 8, Signed)));

  BOOST_CHECK(
      Congruence(Int(1, 8, Signed), Int(0, 8, Signed)).sign_cast(Unsigned) ==
      Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed)).sign_cast(Unsigned) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Signed), Int(1, 8, Signed)).sign_cast(Unsigned) ==
      Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(8, 8, Signed), Int(3, 8, Signed)).sign_cast(Unsigned) ==
      Congruence(Int(8, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(
      Congruence(Int(127, 8, Signed), Int(1, 8, Signed)).sign_cast(Unsigned) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(
      Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).sign_cast(Signed) ==
      Congruence(Int(1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)).sign_cast(Signed) ==
      Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)).sign_cast(Signed) ==
      Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(
      Congruence(Int(8, 8, Unsigned), Int(3, 8, Unsigned)).sign_cast(Signed) ==
      Congruence(Int(8, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(Congruence(Int(254, 8, Unsigned), Int(1, 8, Unsigned))
                  .sign_cast(Signed) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));
}

BOOST_AUTO_TEST_CASE(test_add) {
  BOOST_CHECK(add(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      add(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      add(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(
      add(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(0, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(0, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(-127, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(127, 8, Signed)));
  BOOST_CHECK(
      add(Congruence(Int(127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(-128, 8, Signed)));

  BOOST_CHECK(add(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(add(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(add(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(add(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(add(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(add(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(add(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      add(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      add(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(6, 8, Signed)));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      add(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      add(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      add(Congruence(Int(0, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      add(Congruence(Int(2, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned)));
  BOOST_CHECK(
      add(Congruence(Int(255, 8, Unsigned)), Congruence(Int(0, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      add(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));

  BOOST_CHECK(add(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(add(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(6, 8, Unsigned)));
  BOOST_CHECK(add(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_add_no_wrap) {
  BOOST_CHECK(
      add_no_wrap(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(add_no_wrap(Congruence::bottom(8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(127, 8, Signed))) ==
              Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(-127, 8, Signed)),
                          Congruence(Int(127, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(127, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(3, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(6, 8, Signed), Int(2, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(6, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  // Unsigned

  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Unsigned)),
                          Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(6, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(2, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(3, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned), Int(2, 8, Unsigned)));

  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(6, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sub) {
  BOOST_CHECK(sub(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      sub(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(-2, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(-128, 8, Signed)), Congruence(Int(-127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(-127, 8, Signed)), Congruence(Int(-127, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(-128, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(127, 8, Signed)));
  BOOST_CHECK(
      sub(Congruence(Int(127, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(-128, 8, Signed)));

  BOOST_CHECK(sub(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(sub(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(sub(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      sub(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      sub(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(6, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      sub(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      sub(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned)));
  BOOST_CHECK(
      sub(Congruence(Int(255, 8, Unsigned)), Congruence(Int(0, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      sub(Congruence(Int(0, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));

  BOOST_CHECK(sub(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(sub(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(sub(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(6, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sub_no_wrap) {
  BOOST_CHECK(
      sub_no_wrap(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(sub_no_wrap(Congruence::bottom(8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(-2, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(-127, 8, Signed))) ==
              Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(-127, 8, Signed)),
                          Congruence(Int(-127, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(127, 8, Signed)),
                          Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(3, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(6, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(6, 8, Signed)));

  // Unsigned

  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(10, 8, Unsigned)),
                          Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(6, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(3, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(6, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_mul) {
  BOOST_CHECK(mul(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      mul(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      mul(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(4, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-128, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(-127, 8, Signed)), Congruence(Int(-127, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(127, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(-127, 8, Signed)));
  BOOST_CHECK(
      mul(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(-128, 8, Signed)));

  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(4, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(mul(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(
      mul(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      mul(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence(Int(16, 8, Signed), Int(8, 8, Signed)));
  BOOST_CHECK(mul(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(6, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      mul(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      mul(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      mul(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      mul(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(40, 8, Unsigned)));
  BOOST_CHECK(
      mul(Congruence(Int(255, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      mul(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(254, 8, Unsigned)));

  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(4, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(mul(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(mul(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_mul_no_wrap) {
  BOOST_CHECK(
      mul_no_wrap(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      mul_no_wrap(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(mul_no_wrap(Congruence::bottom(8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      mul_no_wrap(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(mul_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(1, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(127, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(-127, 8, Signed)),
                          Congruence(Int(-127, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(127, 8, Signed)),
                          Congruence(Int(-1, 8, Signed))) ==
              Congruence(Int(-127, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(4, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence(Int(9, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(6, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(6), 8, Signed),
                          Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence(Int(16, 8, Signed), Int(8, 8, Signed)));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(6), 8, Signed),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(6, 8, Signed)));

  // Unsigned

  BOOST_CHECK(mul_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(10, 8, Unsigned)),
                          Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(40, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(2, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(4, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(mul_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      mul_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_div) {
  BOOST_CHECK(div(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      div(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      div(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      div(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(7, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(3, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(-127, 8, Signed)), Congruence(Int(-127, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(127, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(-127, 8, Signed)));
  BOOST_CHECK(
      div(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed))) == Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      div(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      div(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(div(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(div(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(
      div(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      div(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      div(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      div(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      div(Congruence(Int(255, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      div(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(127, 8, Unsigned)));

  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(div(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_rem) {
  BOOST_CHECK(rem(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      rem(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      rem(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      rem(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(7, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(-7, 8, Signed)), Congruence(Int(5, 8, Signed))) ==
      Congruence(Int(-2, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(-127, 8, Signed)), Congruence(Int(-127, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(127, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));

  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(rem(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence(Int(3, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      rem(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      rem(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(rem(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(rem(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(
      rem(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      rem(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      rem(Congruence(Int(255, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));

  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(rem(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(rem(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(rem(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_shl) {
  BOOST_CHECK(shl(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      shl(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      shl(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(8, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(7, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(28, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(-7, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(-56, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(-128, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(-127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(1, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      shl(Congruence(Int(1, 8, Signed)), Congruence(Int(7, 8, Signed))) ==
      Congruence(Int(-128, 8, Signed)));
  BOOST_CHECK(
      shl(Congruence(Int(1, 8, Signed)), Congruence(Int(8, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(4, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(shl(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      shl(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      shl(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(shl(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence(Int(32, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Congruence(Z(128), Z(6), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      shl(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      shl(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned)));
  BOOST_CHECK(
      shl(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(160, 8, Unsigned)));
  BOOST_CHECK(
      shl(Congruence(Int(255, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(254, 8, Unsigned)));
  BOOST_CHECK(
      shl(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(252, 8, Unsigned)));

  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(4, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(14, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(128, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_shl_no_wrap) {
  BOOST_CHECK(
      shl_no_wrap(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(shl_no_wrap(Congruence::bottom(8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(shl_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(1, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(7, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(28, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(-7, 8, Signed)),
                          Congruence(Int(3, 8, Signed))) ==
              Congruence(Int(-56, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(-128, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(-127, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(1, 8, Signed)),
                          Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(1, 8, Signed)),
                          Congruence(Int(7, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(1, 8, Signed)),
                          Congruence(Int(8, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence::top(8, Signed)) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(4, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                          Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence(Int(3, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                          Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(6, 8, Signed), Int(2, 8, Signed)));

  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Signed),
                          Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(6), 8, Signed),
                          Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence(Int(32, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(6), 8, Signed),
                          Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));

  // Unsigned

  BOOST_CHECK(shl_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(10, 8, Unsigned)),
                          Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(160, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(255, 8, Unsigned)),
                          Congruence(Int(2, 8, Unsigned))) ==
              Congruence::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence::top(8, Unsigned)) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(4, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(6, 8, Unsigned), Int(2, 8, Unsigned)));

  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                          Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(14, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence(Int(128, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl_no_wrap(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_lshr) {
  BOOST_CHECK(lshr(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      lshr(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      lshr(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(7, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(-7, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(31, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(-128, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(-127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(1, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      lshr(Congruence(Int(1, 8, Signed)), Congruence(Int(7, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr(Congruence(Int(1, 8, Signed)), Congruence(Int(8, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(6), 8, Signed),
                   Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(6), 8, Signed),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(
      lshr(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(63, 8, Unsigned)));

  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_lshr_exact) {
  BOOST_CHECK(
      lshr_exact(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(lshr_exact(Congruence::bottom(8, Signed),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(lshr_exact(Congruence(Int(0, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(7, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(-7, 8, Signed)),
                         Congruence(Int(3, 8, Signed))) ==
              Congruence(Int(31, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(-128, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(64, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(-127, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(64, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(7, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(8, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                         Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(6), 8, Signed),
                         Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(6), 8, Signed),
                         Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(lshr_exact(Congruence(Int(0, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Int(0, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(10, 8, Unsigned)),
                         Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(255, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(255, 8, Unsigned)),
                         Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(63, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                 Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                 Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                 Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                 Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                 Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                 Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(lshr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                 Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                 Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_ashr) {
  BOOST_CHECK(ashr(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      ashr(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      ashr(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(1, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(2, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(7, 8, Signed)), Congruence(Int(2, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(-7, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(-128, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(-127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(1, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      ashr(Congruence(Int(1, 8, Signed)), Congruence(Int(7, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr(Congruence(Int(1, 8, Signed)), Congruence(Int(8, 8, Signed))) ==
      Congruence::bottom(8, Signed));

  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(6), 8, Signed),
                   Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(6), 8, Signed),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(
      ashr(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Congruence(Int(2, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Congruence(Int(10, 8, Unsigned)), Congruence(Int(4, 8, Unsigned))) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(ashr(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));

  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_ashr_exact) {
  BOOST_CHECK(
      ashr_exact(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::top(8, Signed));

  BOOST_CHECK(ashr_exact(Congruence::bottom(8, Signed),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
      Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(ashr_exact(Congruence(Int(0, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(7, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(-7, 8, Signed)),
                         Congruence(Int(3, 8, Signed))) ==
              Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(-128, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(-64, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(-127, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence(Int(-64, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(-1, 8, Signed))) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(7, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(1, 8, Signed)),
                         Congruence(Int(8, 8, Signed))) ==
              Congruence::bottom(8, Signed));

  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(3, 8, Signed), Int(0, 8, Signed)),
                         Congruence(Int(3, 8, Signed), Int(0, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                         Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Signed),
                         Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(6), 8, Signed),
                         Congruence(Int(8, 8, Signed), Int(4, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(6), 8, Signed),
                         Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));

  // Unsigned

  BOOST_CHECK(ashr_exact(Congruence(Int(0, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Int(0, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(10, 8, Unsigned)),
                         Congruence(Int(4, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(255, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(255, 8, Unsigned)),
                         Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                 Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                 Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                 Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                 Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                 Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                 Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));

  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(ashr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                         Congruence(Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                 Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Congruence(Z(128), Z(7), 8, Unsigned),
                 Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
      Congruence::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK(and_(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      and_(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      and_(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(2, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(1, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(-127, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(128, 8, Signed)));
  BOOST_CHECK(
      and_(Congruence(Int(127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(1, 8, Signed)));

  BOOST_CHECK(and_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(and_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(and_(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(1, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      and_(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      and_(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Congruence(Int(0, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Congruence(Int(2, 8, Unsigned)), Congruence(Int(3, 8, Unsigned))) ==
      Congruence(Int(2, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned)));

  BOOST_CHECK(and_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(and_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned)));
  BOOST_CHECK(and_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK(or_(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      or_(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      or_(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(
      or_(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(2, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(3, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(1, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(3, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(-127, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      or_(Congruence(Int(127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(127, 8, Signed)));

  BOOST_CHECK(or_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                  Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(or_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(or_(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                  Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(
      or_(Congruence(Z(128), Z(7), 8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(
      or_(Congruence(Z(128), Z(7), 8, Signed), Congruence(Int(0, 8, Signed))) ==
      Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Signed),
                  Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(3, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      or_(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      or_(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      or_(Congruence(Int(0, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      or_(Congruence(Int(2, 8, Unsigned)), Congruence(Int(3, 8, Unsigned))) ==
      Congruence(Int(3, 8, Unsigned)));
  BOOST_CHECK(
      or_(Congruence(Int(255, 8, Unsigned)), Congruence(Int(0, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      or_(Congruence(Int(255, 8, Unsigned)), Congruence(Int(2, 8, Unsigned))) ==
      Congruence(Int(255, 8, Unsigned)));

  BOOST_CHECK(or_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                  Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(or_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                  Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned)));
  BOOST_CHECK(or_(Congruence(Z(128), Z(7), 8, Unsigned),
                  Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK(xor_(Congruence::top(8, Signed), Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence::top(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));

  BOOST_CHECK(
      xor_(Congruence::bottom(8, Signed), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence::bottom(8, Signed), Congruence::top(8, Signed)) ==
              Congruence::bottom(8, Signed));

  // Signed

  BOOST_CHECK(
      xor_(Congruence(Int(0, 8, Signed)), Congruence::bottom(8, Signed)) ==
      Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(0, 8, Signed)), Congruence::top(8, Signed)) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(
      xor_(Congruence(Int(0, 8, Signed)), Congruence(Int(0, 8, Signed))) ==
      Congruence(Int(0, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(2, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(3, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(1, 8, Signed)), Congruence(Int(3, 8, Signed))) ==
      Congruence(Int(2, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(-128, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-1, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(-127, 8, Signed)), Congruence(Int(127, 8, Signed))) ==
      Congruence(Int(-2, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(-128, 8, Signed)), Congruence(Int(-1, 8, Signed))) ==
      Congruence(Int(127, 8, Signed)));
  BOOST_CHECK(
      xor_(Congruence(Int(127, 8, Signed)), Congruence(Int(1, 8, Signed))) ==
      Congruence(Int(126, 8, Signed)));

  BOOST_CHECK(xor_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Signed), Int(0, 8, Signed)),
                   Congruence(Int(2, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(xor_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(8, 8, Signed), Int(2, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Signed), Int(2, 8, Signed)),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(3, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Int(3, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(3, 8, Signed), Int(1, 8, Signed))) ==
              Congruence::top(8, Signed));
  BOOST_CHECK(xor_(Congruence(Int(6, 8, Signed), Int(1, 8, Signed)),
                   Congruence(Int(6, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::bottom(8, Signed)) ==
              Congruence::bottom(8, Signed));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence::top(8, Signed)) == Congruence::top(8, Signed));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(0, 8, Signed))) ==
              Congruence(Z(128), Z(7), 8, Signed));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(8, 8, Signed), Int(7, 8, Signed))) ==
              Congruence(Int(8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Signed),
                   Congruence(Int(4, 8, Signed), Int(1, 8, Signed))) ==
              Congruence(Int(4, 8, Signed), Int(2, 8, Signed)));

  // Unsigned

  BOOST_CHECK(
      xor_(Congruence(Int(0, 8, Unsigned)), Congruence::bottom(8, Unsigned)) ==
      Congruence::bottom(8, Unsigned));
  BOOST_CHECK(
      xor_(Congruence(Int(0, 8, Unsigned)), Congruence::top(8, Unsigned)) ==
      Congruence::top(8, Unsigned));
  BOOST_CHECK(
      xor_(Congruence(Int(0, 8, Unsigned)), Congruence(Int(1, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Congruence(Int(2, 8, Unsigned)), Congruence(Int(3, 8, Unsigned))) ==
      Congruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(255, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(255, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned))) ==
              Congruence(Int(253, 8, Unsigned)));

  BOOST_CHECK(xor_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)),
                   Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(xor_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(8, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(3, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Int(6, 8, Unsigned), Int(1, 8, Unsigned)),
                   Congruence(Int(12, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::bottom(8, Unsigned)) ==
              Congruence::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence::top(8, Unsigned)) ==
              Congruence::top(8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(0, 8, Unsigned))) ==
              Congruence(Z(128), Z(7), 8, Unsigned));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(8, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Congruence(Int(8, 8, Unsigned), Int(0, 8, Unsigned)));
  BOOST_CHECK(xor_(Congruence(Z(128), Z(7), 8, Unsigned),
                   Congruence(Int(4, 8, Unsigned), Int(1, 8, Unsigned))) ==
              Congruence(Int(4, 8, Unsigned), Int(2, 8, Unsigned)));
}
