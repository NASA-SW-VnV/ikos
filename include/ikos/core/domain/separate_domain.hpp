/*******************************************************************************
 *
 * \file
 * \brief Generic implementation of non-relational domains
 *
 * Author: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#pragma once

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/domain/abstract_domain.hpp>

namespace ikos {
namespace core {

/// \brief Generic implementation of non-relational domains
template < typename Key, typename Value >
class SeparateDomain final
    : public AbstractDomain< SeparateDomain< Key, Value > > {
public:
  static_assert(IsAbstractDomain< Value >::value,
                "Value must implement AbstractDomain");

private:
  using PatriciaTreeMapT = PatriciaTreeMap< Key, Value >;

public:
  using Iterator = typename PatriciaTreeMapT::Iterator;

private:
  PatriciaTreeMapT _tree;
  bool _is_bottom;

private:
  struct TopTag {};
  struct BottomTag {};
  struct BottomFound {};

  /// \brief Create the top abstract value
  explicit SeparateDomain(TopTag) : _is_bottom(false) {}

  /// \brief Create the bottom abstract value
  explicit SeparateDomain(BottomTag) : _is_bottom(true) {}

public:
  /// \brief Create the top abstract value
  static SeparateDomain top() { return SeparateDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static SeparateDomain bottom() { return SeparateDomain(BottomTag{}); }

  /// \brief Copy constructor
  SeparateDomain(const SeparateDomain&) noexcept = default;

  /// \brief Move constructor
  SeparateDomain(SeparateDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  SeparateDomain& operator=(const SeparateDomain&) noexcept = default;

  /// \brief Move assignment operator
  SeparateDomain& operator=(SeparateDomain&&) noexcept = default;

  /// \brief Destructor
  ~SeparateDomain() override = default;

  /// \brief Begin iterator over the pairs (key, value)
  Iterator begin() const {
    ikos_assert(!this->is_bottom());
    return this->_tree.begin();
  }

  /// \brief End iterator over the pairs (key, value)
  Iterator end() const {
    ikos_assert(!this->is_bottom());
    return this->_tree.end();
  }

  void normalize() override {}

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override {
    return !this->is_bottom() && this->_tree.empty();
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_tree.clear();
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_tree.clear();
  }

  bool leq(const SeparateDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.leq(other._tree, [](const Value& x, const Value& y) {
        return x.leq(y);
      });
    }
  }

  bool equals(const SeparateDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.equals(other._tree,
                                [](const Value& x, const Value& y) {
                                  return x.equals(y);
                                });
    }
  }

  void join_with(const SeparateDomain& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const Value& x, const Value& y) {
                                   Value z = x.join(y);
                                   if (z.is_top()) {
                                     return boost::optional< Value >(
                                         boost::none);
                                   }
                                   return boost::optional< Value >(z);
                                 });
    }
  }

  void join_loop_with(const SeparateDomain& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const Value& x, const Value& y) {
                                   Value z = x.join_loop(y);
                                   if (z.is_top()) {
                                     return boost::optional< Value >(
                                         boost::none);
                                   }
                                   return boost::optional< Value >(z);
                                 });
    }
  }

  void join_iter_with(const SeparateDomain& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const Value& x, const Value& y) {
                                   Value z = x.join_loop(y);
                                   if (z.is_top()) {
                                     return boost::optional< Value >(
                                         boost::none);
                                   }
                                   return boost::optional< Value >(z);
                                 });
    }
  }

  void widen_with(const SeparateDomain& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const Value& x, const Value& y) {
                                   Value z = x.widening(y);
                                   if (z.is_top()) {
                                     return boost::optional< Value >(
                                         boost::none);
                                   }
                                   return boost::optional< Value >(z);
                                 });
    }
  }

  void meet_with(const SeparateDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree, [](const Value& x, const Value& y) {
          Value z = x.meet(y);
          if (z.is_bottom()) {
            throw BottomFound();
          }
          return boost::optional< Value >(z);
        });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  void narrow_with(const SeparateDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree, [](const Value& x, const Value& y) {
          Value z = x.narrowing(y);
          if (z.is_bottom()) {
            throw BottomFound();
          }
          return boost::optional< Value >(z);
        });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Set the abstract value of the given key
  void set(const Key& key, const Value& value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else if (value.is_top()) {
      this->_tree.erase(key);
    } else {
      this->_tree.insert_or_assign(key, value);
    }
  }

  /// \brief Refine the abstract value of the given key
  void refine(const Key& key, const Value& value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else if (value.is_top()) {
      return;
    } else {
      try {
        this->_tree.update_or_insert(
            [](const Value& x, const Value& y) {
              Value z = x.meet(y);
              if (z.is_bottom()) {
                throw BottomFound();
              }
              return boost::optional< Value >(z);
            },
            key,
            value);
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Forget the abstract value of the given key
  void forget(const Key& key) {
    if (this->is_bottom()) {
      return;
    }
    this->_tree.erase(key);
  }

  /// \brief Get the abstract value for the given key
  Value get(const Key& key) const {
    if (this->is_bottom()) {
      return Value::bottom();
    } else {
      boost::optional< const Value& > v = this->_tree.at(key);
      if (v) {
        return *v;
      } else {
        return Value::top();
      }
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      this->_tree.dump(o);
    }
  }

  static std::string name() { return "separate domain of " + Value::name(); }

}; // end class SeparateDomain

} // end namespace core
} // end namespace ikos
