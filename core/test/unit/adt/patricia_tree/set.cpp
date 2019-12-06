/*******************************************************************************
 *
 * Tests for PatriciaTreeSet
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

#define BOOST_TEST_MODULE test_patricia_tree_set
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/adt/patricia_tree/set.hpp>

BOOST_AUTO_TEST_CASE(test_patricia_tree_set) {
  using Index = ikos::core::Index;
  using Set = ikos::core::PatriciaTreeSet< Index >;
  Set s;

  // test insert
  BOOST_CHECK(s.empty());
  BOOST_CHECK(s.size() == 0);
  BOOST_CHECK(!s.contains(1));

  s.insert(1);
  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 1);
  BOOST_CHECK(s.contains(1));
  BOOST_CHECK(!s.contains(2));

  s.insert(2);
  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 2);
  BOOST_CHECK(s.contains(1));
  BOOST_CHECK(s.contains(2));
  BOOST_CHECK(!s.contains(3));

  // insert 50, 51, ..., 99
  for (std::size_t i = 50; i < 100; i++) {
    s.insert(i);
  }

  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 52);

  // insert 3, 4, ..., 49
  for (std::size_t i = 3; i < 50; i++) {
    s.insert(i);
  }

  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 99);
  BOOST_CHECK(s.contains(1));
  BOOST_CHECK(s.contains(2));
  for (std::size_t i = 3; i < 100; i++) {
    BOOST_CHECK(s.contains(i));
  }
  BOOST_CHECK(!s.contains(0));
  BOOST_CHECK(!s.contains(100));

  // test erase
  s.erase(0);
  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 99);

  s.erase(1);
  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 98);
  BOOST_CHECK(!s.contains(1));
  BOOST_CHECK(s.contains(2));
  for (std::size_t i = 3; i < 100; i++) {
    BOOST_CHECK(s.contains(i));
  }

  // erase 2, 3, ..., 99
  for (std::size_t i = 2; i < 100; i++) {
    s.erase(i);
  }

  BOOST_CHECK(s.empty());
  BOOST_CHECK(s.size() == 0);

  // test clear
  s.insert(1);
  s.insert(2);
  BOOST_CHECK(!s.empty());
  BOOST_CHECK(s.size() == 2);

  s.clear();
  BOOST_CHECK(s.empty());
  BOOST_CHECK(s.size() == 0);

  // test begin/end
  s.clear();
  s.insert(1);
  {
    auto it = s.begin();
    BOOST_CHECK((it != s.end()));
    BOOST_CHECK(*it == 1);
    ++it;
    BOOST_CHECK((it == s.end()));
  }

  s.insert(2);
  BOOST_CHECK(s.equals(Set({1, 2})));

  // insert 50, 51, ..., 99
  for (std::size_t i = 50; i < 100; i++) {
    s.insert(i);
  }

  // insert 3, 4, ..., 49
  for (std::size_t i = 3; i < 50; i++) {
    s.insert(i);
  }

  BOOST_CHECK(std::distance(s.begin(), s.end()) == 99);

  // test leq
  Set s1;
  Set s2;
  s1.insert(1);
  s2.insert(1);
  s2.insert(2);
  BOOST_CHECK(s1.is_subset_of(s2));
  BOOST_CHECK(!s2.is_subset_of(s1));

  s1.insert(3);
  BOOST_CHECK(!s1.is_subset_of(s2));

  // test equals
  s1.clear();
  s2.clear();
  BOOST_CHECK(s1.equals(s2));
  s1.insert(1);
  BOOST_CHECK(!s1.equals(s2));
  s2.insert(1);
  BOOST_CHECK(s1.equals(s2));
  s1.insert(2);
  s2.insert(2);
  BOOST_CHECK(s1.equals(s2));

  // test join
  s1.clear();
  s1.insert(1);
  s1.insert(2);
  s2.clear();
  s2.insert(1);
  BOOST_CHECK(s1.equals(s1.join(s2)));

  s1.clear();
  s1.insert(2);
  s2.clear();
  s2.insert(1);
  BOOST_CHECK(s1.join(s2).equals(Set({1, 2})));

  // test intersect
  s1.clear();
  s1.insert(1);
  s1.insert(2);
  s2.clear();
  s2.insert(1);
  BOOST_CHECK(s1.intersect(s2).equals(Set({1})));
}
