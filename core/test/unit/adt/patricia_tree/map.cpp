/*******************************************************************************
 *
 * Tests for PatriciaTreeMap
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

#define BOOST_TEST_MODULE test_patricia_tree_map
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/adt/patricia_tree/map.hpp>

BOOST_AUTO_TEST_CASE(test_patricia_tree_map) {
  using Index = ikos::core::Index;
  using Map = ikos::core::PatriciaTreeMap< Index, std::string >;
  Map m;

  // test insert_or_assign
  BOOST_CHECK(m.empty());
  BOOST_CHECK(m.size() == 0);
  BOOST_CHECK(!m.at(1));

  m.insert_or_assign(1, "hello");
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 1);
  {
    auto x = m.at(1);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "hello");
  }
  {
    auto x = m.at(2);
    BOOST_CHECK(!x);
  }

  m.insert_or_assign(2, "world");
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 2);
  {
    auto x = m.at(1);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "hello");
  }
  {
    auto x = m.at(2);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "world");
  }
  {
    auto x = m.at(3);
    BOOST_CHECK(!x);
  }

  m.insert_or_assign(1, "bye");
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 2);
  {
    auto x = m.at(1);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "bye");
  }
  {
    auto x = m.at(2);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "world");
  }
  {
    auto x = m.at(3);
    BOOST_CHECK(!x);
  }

  // insert 50, 51, ..., 99
  for (std::size_t i = 50; i < 100; i++) {
    m.insert_or_assign(i, std::to_string(i));
  }

  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 52);

  // insert 3, 4, ..., 49
  for (std::size_t i = 3; i < 50; i++) {
    m.insert_or_assign(i, std::to_string(i));
  }

  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 99);
  {
    auto x = m.at(1);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "bye");
  }
  {
    auto x = m.at(2);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "world");
  }
  for (std::size_t i = 3; i < 100; i++) {
    auto x = m.at(i);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == std::to_string(i));
  }
  {
    auto x = m.at(0);
    BOOST_CHECK(!x);
  }
  {
    auto x = m.at(100);
    BOOST_CHECK(!x);
  }

  // test erase
  m.erase(0);
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 99);

  m.erase(1);
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 98);
  {
    auto x = m.at(1);
    BOOST_CHECK(!x);
  }
  {
    auto x = m.at(2);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == "world");
  }
  for (std::size_t i = 3; i < 100; i++) {
    auto x = m.at(i);
    BOOST_CHECK(!!x);
    BOOST_CHECK(*x == std::to_string(i));
  }

  // erase 2, 3, ..., 99
  for (std::size_t i = 2; i < 100; i++) {
    m.erase(i);
  }

  BOOST_CHECK(m.empty());
  BOOST_CHECK(m.size() == 0);

  // test clear
  m.insert_or_assign(1, "hello");
  m.insert_or_assign(2, "world");
  BOOST_CHECK(!m.empty());
  BOOST_CHECK(m.size() == 2);

  m.clear();
  BOOST_CHECK(m.empty());
  BOOST_CHECK(m.size() == 0);

  // test begin/end
  m.clear();
  m.insert_or_assign(1, "hello");
  {
    auto it = m.begin();
    BOOST_CHECK((it != m.end()));
    BOOST_CHECK(it->first == 1);
    BOOST_CHECK(it->second == "hello");
    ++it;
    BOOST_CHECK((it == m.end()));
  }

  m.insert_or_assign(2, "world");
  const std::array< std::pair< Index, std::string >, 2 > tab = {
      {{2, "world"}, {1, "hello"}}};
  BOOST_CHECK(std::equal(m.begin(), m.end(), std::begin(tab), std::end(tab)));

  // insert 50, 51, ..., 99
  for (std::size_t i = 50; i < 100; i++) {
    m.insert_or_assign(i, std::to_string(i));
  }

  // insert 3, 4, ..., 49
  for (std::size_t i = 3; i < 50; i++) {
    m.insert_or_assign(i, std::to_string(i));
  }

  BOOST_CHECK(std::distance(m.begin(), m.end()) == 99);

  // test transform
  m.clear();
  m.insert_or_assign(1, "hello");
  m.insert_or_assign(2, "world");
  m.transform([](Index, const std::string& s) { return s.substr(0, 1); });

  const std::array< std::pair< Index, std::string >, 2 > tab2 = {
      {{2, "w"}, {1, "h"}}};
  BOOST_CHECK(std::equal(m.begin(), m.end(), std::begin(tab2), std::end(tab2)));

  // test leq
  Map m1;
  Map m2;
  m1.insert_or_assign(1, "hello");
  m1.insert_or_assign(2, "world");
  m2.insert_or_assign(1, "zzzzz");
  BOOST_CHECK(m1.leq(m2, std::less<>()));
  BOOST_CHECK(!m2.leq(m1, std::less<>()));

  m2.insert_or_assign(3, "test");
  BOOST_CHECK(!m1.leq(m2, std::less<>()));

  // test equals
  m1.clear();
  m2.clear();
  BOOST_CHECK(m1.equals(m2, std::equal_to<>()));
  m1.insert_or_assign(1, "hello");
  BOOST_CHECK(!m1.equals(m2, std::equal_to<>()));
  m2.insert_or_assign(1, "hello");
  BOOST_CHECK(m1.equals(m2, std::equal_to<>()));
  m1.insert_or_assign(2, "a");
  m2.insert_or_assign(2, "b");
  BOOST_CHECK(!m1.equals(m2, std::equal_to<>()));

  // test join
  m1.clear();
  m1.insert_or_assign(1, "hello");
  m1.insert_or_assign(2, "world");
  m2.clear();
  m2.insert_or_assign(1, "zzzzz");
  m = m1.join(m2, std::plus<>());
  const std::array< std::pair< Index, std::string >, 2 > tab3 = {
      {{2, "world"}, {1, "hellozzzzz"}}};
  BOOST_CHECK(std::equal(m.begin(), m.end(), std::begin(tab3), std::end(tab3)));

  // test intersect
  m1.clear();
  m1.insert_or_assign(1, "hello");
  m1.insert_or_assign(2, "world");
  m2.clear();
  m2.insert_or_assign(1, "zzzzz");
  m = m1.intersect(m2, std::plus<>());
  const std::array< std::pair< Index, std::string >, 1 > tab4 = {
      {{1, "hellozzzzz"}}};
  BOOST_CHECK(std::equal(m.begin(), m.end(), std::begin(tab4), std::end(tab4)));
}
