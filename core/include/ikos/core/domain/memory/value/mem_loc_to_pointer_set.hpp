/*******************************************************************************
 *
 * \file
 * \brief Map from memory locations to set of pointers
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

#pragma once

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/value/pointer/pointer_set.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Map from memory locations to set of pointers
template < typename MemoryLocationRef >
class MemLocToPointerSet final
    : public core::AbstractDomain< MemLocToPointerSet< MemoryLocationRef > > {
public:
  static_assert(IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

private:
  using PointerSetT = PointerSet< MemoryLocationRef >;
  using PatriciaTreeMapT = PatriciaTreeMap< MemoryLocationRef, PointerSetT >;

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
  explicit MemLocToPointerSet(TopTag) : _is_bottom(false) {}

  /// \brief Create the bottom abstract value
  explicit MemLocToPointerSet(BottomTag) : _is_bottom(true) {}

public:
  /// \brief Create the top abstract value
  static MemLocToPointerSet top() { return MemLocToPointerSet(TopTag{}); }

  /// \brief Create the bottom abstract value
  static MemLocToPointerSet bottom() { return MemLocToPointerSet(BottomTag{}); }

  /// \brief Copy constructor
  MemLocToPointerSet(const MemLocToPointerSet&) noexcept = default;

  /// \brief Move constructor
  MemLocToPointerSet(MemLocToPointerSet&&) noexcept = default;

  /// \brief Copy assignment operator
  MemLocToPointerSet& operator=(const MemLocToPointerSet&) noexcept = default;

  /// \brief Move assignment operator
  MemLocToPointerSet& operator=(MemLocToPointerSet&&) noexcept = default;

  /// \brief Destructor
  ~MemLocToPointerSet() override = default;

  /// \brief Begin iterator over the pairs (variable, value)
  Iterator begin() const {
    ikos_assert(!this->is_bottom());
    return this->_tree.begin();
  }

  /// \brief End iterator over the pairs (variable, value)
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

  bool leq(const MemLocToPointerSet& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.leq(other._tree,
                             [](const PointerSetT& x, const PointerSetT& y) {
                               return x.leq(y);
                             });
    }
  }

  bool equals(const MemLocToPointerSet& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.equals(other._tree,
                                [](const PointerSetT& x, const PointerSetT& y) {
                                  return x.equals(y);
                                });
    }
  }

  void join_with(const MemLocToPointerSet& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const PointerSetT& x,
                                    const PointerSetT& y) {
                                   PointerSetT z = x.join(y);
                                   if (z.is_top()) {
                                     return boost::optional< PointerSetT >(
                                         boost::none);
                                   }
                                   return boost::optional< PointerSetT >(z);
                                 });
    }
  }

  void join_loop_with(const MemLocToPointerSet& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const PointerSetT& x,
                                    const PointerSetT& y) {
                                   PointerSetT z = x.join_loop(y);
                                   if (z.is_top()) {
                                     return boost::optional< PointerSetT >(
                                         boost::none);
                                   }
                                   return boost::optional< PointerSetT >(z);
                                 });
    }
  }

  void join_iter_with(const MemLocToPointerSet& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const PointerSetT& x,
                                    const PointerSetT& y) {
                                   PointerSetT z = x.join_iter(y);
                                   if (z.is_top()) {
                                     return boost::optional< PointerSetT >(
                                         boost::none);
                                   }
                                   return boost::optional< PointerSetT >(z);
                                 });
    }
  }

  void widen_with(const MemLocToPointerSet& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const PointerSetT& x,
                                    const PointerSetT& y) {
                                   PointerSetT z = x.widening(y);
                                   if (z.is_top()) {
                                     return boost::optional< PointerSetT >(
                                         boost::none);
                                   }
                                   return boost::optional< PointerSetT >(z);
                                 });
    }
  }

  void widen_threshold_with(const MemLocToPointerSet& other,
                            const MachineInt& threshold) {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [threshold](const PointerSetT& x,
                                             const PointerSetT& y) {
                                   PointerSetT z =
                                       x.widening_threshold(y, threshold);
                                   if (z.is_top()) {
                                     return boost::optional< PointerSetT >(
                                         boost::none);
                                   }
                                   return boost::optional< PointerSetT >(z);
                                 });
    }
  }

  void meet_with(const MemLocToPointerSet& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [](const PointerSetT& x, const PointerSetT& y) {
                                PointerSetT z = x.meet(y);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< PointerSetT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  void narrow_with(const MemLocToPointerSet& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [](const PointerSetT& x, const PointerSetT& y) {
                                PointerSetT z = x.narrowing(y);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< PointerSetT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  void narrow_threshold_with(const MemLocToPointerSet& other,
                             const MachineInt& threshold) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [threshold](const PointerSetT& x,
                                          const PointerSetT& y) {
                                PointerSetT z =
                                    x.narrowing_threshold(y, threshold);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< PointerSetT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Get the pointer set for the given memory location
  PointerSetT get(MemoryLocationRef addr,
                  uint64_t bit_width,
                  Signedness sign) const {
    if (this->is_bottom()) {
      return PointerSetT::bottom(bit_width, sign);
    } else {
      boost::optional< const PointerSetT& > v = this->_tree.at(addr);
      if (v) {
        return *v;
      } else {
        return PointerSetT::top(bit_width, sign);
      }
    }
  }

  /// \brief Set the pointer set of the given memory location
  void set(MemoryLocationRef addr, const PointerSetT& pointer_set) {
    if (this->is_bottom()) {
      return;
    } else if (pointer_set.is_bottom()) {
      this->set_to_bottom();
    } else if (pointer_set.is_top()) {
      this->_tree.erase(addr);
    } else {
      this->_tree.insert_or_assign(addr, pointer_set);
    }
  }

  /// \brief Refine the pointer set of the given memory location
  void refine(MemoryLocationRef addr, const PointerSetT& pointer_set) {
    if (this->is_bottom()) {
      return;
    } else if (pointer_set.is_bottom()) {
      this->set_to_bottom();
    } else if (pointer_set.is_top()) {
      return;
    } else {
      try {
        this->_tree.update_or_insert(
            [](const PointerSetT& x, const PointerSetT& y) {
              PointerSetT z = x.meet(y);
              if (z.is_bottom()) {
                throw BottomFound();
              }
              return boost::optional< PointerSetT >(z);
            },
            addr,
            pointer_set);
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Forget the pointer set of the given memory location
  void forget(MemoryLocationRef addr) {
    if (this->is_bottom()) {
      return;
    }
    this->_tree.erase(addr);
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      this->_tree.dump(o);
    }
  }

  static std::string name() { return "memory location to pointer set"; }

}; // end class MemLocToPointerSet

} // end namespace memory
} // end namespace core
} // end namespace ikos
