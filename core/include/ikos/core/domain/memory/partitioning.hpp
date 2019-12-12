/*******************************************************************************
 *
 * \file
 * \brief A memory abstract domain with partitioning
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <algorithm>

#include <boost/optional.hpp>

#include <ikos/core/adt/small_vector.hpp>

#include <ikos/core/domain/memory/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Partitioning memory abstract domain
///
/// This class implements a memory abstract domain with partitioning.
///
/// The partitioning is performed on the valuations of a given integer
/// variable.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename MemoryDomain >
class PartitioningDomain final
    : public memory::AbstractDomain<
          VariableRef,
          MemoryLocationRef,
          PartitioningDomain< VariableRef, MemoryLocationRef, MemoryDomain > > {
public:
  static_assert(memory::IsAbstractDomain< MemoryDomain,
                                          VariableRef,
                                          MemoryLocationRef >::value,
                "MemoryDomain must implement memory::AbstractDomain");

public:
  using IntUnaryOperator = machine_int::UnaryOperator;
  using IntBinaryOperator = machine_int::BinaryOperator;
  using IntPredicate = machine_int::Predicate;
  using IntLinearExpression = LinearExpression< MachineInt, VariableRef >;
  using IntInterval = machine_int::Interval;
  using IntCongruence = machine_int::Congruence;
  using IntIntervalCongruence = machine_int::IntervalCongruence;
  using PointerPredicate = pointer::Predicate;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointerSetT = PointerSet< MemoryLocationRef >;
  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

private:
  using IntVariableTrait = machine_int::VariableTraits< VariableRef >;
  using ScalarVariableTrait = scalar::VariableTraits< VariableRef >;

private:
  /// \brief Partition
  struct Partition {
    /// \brief Interval of the partitioning variable
    IntInterval interval;

    /// \brief Memory domain abstract value
    MemoryDomain memory;
  };

private:
  /// \brief Partitioning variable, or boost::none
  boost::optional< VariableRef > _variable;

  /// \brief List of partitions, ordered by intervals
  core::SmallVector< Partition, 1 > _partitions;

  // Invariants:
  // * _partitions.size() >= 1
  // * ∀i, !_partitions[i].interval.is_bottom()
  // * ∀i, _partitions[i].interval.ub() < _partitions[i + 1].interval.lb()
  // * _variable == boost::none =>
  //     _partitions.size() == 1 &&
  //     _partitions[0].interval == IntInterval::top(1, Signed)
  // * _variable != boost::none =>
  //     ∀i, partition = _partitions[i],
  //     partition.memory.int_to_interval(_variable).leq(partition.interval)
  //
  // After normalization:
  // * ∀i > 1, !_partitions[i].memory.is_bottom()

public:
  /// \brief Create an abstract value with the given underlying memory domain
  PartitioningDomain(MemoryDomain memory)
      : _variable(boost::none),
        _partitions{Partition{IntInterval::top(1, Signed), std::move(memory)}} {
  }

  /// \brief Copy constructor
  PartitioningDomain(const PartitioningDomain&) = default;

  /// \brief Move constructor
  PartitioningDomain(PartitioningDomain&&) = default;

  /// \brief Copy assignment operator
  PartitioningDomain& operator=(const PartitioningDomain&) = default;

  /// \brief Move assignment operator
  PartitioningDomain& operator=(PartitioningDomain&&) = default;

  /// \brief Destructor
  ~PartitioningDomain() override = default;

  /// \name Partitioning abstract domain methods
  /// @{

  void normalize() override {
    ikos_assert(this->_partitions.size() >= 1);

    // Remove bottom partitions, but always keep at least one partition.
    // Start from the end for efficiency.
    for (auto it = this->_partitions.end();
         it != this->_partitions.begin() && this->_partitions.size() > 1;) {
      --it;
      it->memory.normalize();
      if (it->memory.is_bottom()) {
        it = this->_partitions.erase(it);
      }
    }

    // Normalize the first partition
    this->_partitions[0].memory.normalize();
  }

private:
  /// \brief Join the partitions and return the merged partition
  Partition join_partitions() const {
    Partition p = this->_partitions[0];

    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      p.interval.join_with(it->interval);
      p.memory.join_with(it->memory);
    }

    return p;
  }

  /// \brief Update the partitions after a variable update
  void update_partitions() {
    if (!this->_variable) {
      return;
    }

    // Update intervals and remove bottom partitions.
    // Start from the end for efficiency.
    for (auto it = this->_partitions.end(); it != this->_partitions.begin();) {
      --it;
      it->memory.normalize();
      IntInterval interval = it->memory.int_to_interval(*this->_variable);

      if (interval.is_bottom()) {
        if (this->_partitions.size() > 1) {
          it = this->_partitions.erase(it);
        } else {
          it->interval.set_to_top();
          return;
        }
      } else {
        it->interval = std::move(interval);
      }
    }

    // Sort partitions by interval lower bounds
    std::sort(this->_partitions.begin(),
              this->_partitions.end(),
              [](const Partition& a, const Partition& b) {
                return a.interval.lb() < b.interval.lb();
              });

    // Merge partitions if necessary.
    // Start from the end for efficiency.
    for (auto it = std::prev(this->_partitions.end());
         it != this->_partitions.begin() && this->_partitions.size() > 1;) {
      --it;
      if (it->interval.ub() >= std::next(it)->interval.lb()) {
        it->interval.join_with(std::next(it)->interval);
        it->memory.join_with(std::move(std::next(it)->memory));
        it = std::prev(this->_partitions.erase(std::next(it)));
      }
    }
  }

  /// \brief Return True if both abstract values have the same partitioning
  bool is_same_partitioning(const PartitioningDomain& other) const {
    return this->_variable == other._variable &&
           this->_partitions.size() == other._partitions.size() &&
           std::equal(this->_partitions.begin(),
                      this->_partitions.end(),
                      other._partitions.begin(),
                      other._partitions.end(),
                      [](const Partition& a, const Partition& b) {
                        return a.interval == b.interval;
                      });
  }

public:
  void partitioning_set_variable(VariableRef x) override {
    if (!ScalarVariableTrait::is_int(x)) {
      this->partitioning_disable();
    } else if (this->_variable && *this->_variable == x) {
      return;
    } else {
      this->_variable = x;
      this->update_partitions();
    }
  }

  boost::optional< VariableRef > partitioning_variable() const override {
    return this->_variable;
  }

  void partitioning_join() override {
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      this->_partitions[0].interval.join_with(it->interval);
      this->_partitions[0].memory.join_with(std::move(it->memory));
    }

    this->_partitions.erase(std::next(this->_partitions.begin()),
                            this->_partitions.end());
  }

  void partitioning_disable() override {
    if (!this->_variable) {
      // Already done
      ikos_assert(this->_partitions.size() == 1);
      return;
    }

    this->partitioning_join();
    this->_variable = boost::none;
    this->_partitions[0].interval = IntInterval::top(1, Signed);
  }

  /// @}
  /// \name Implement core abstract domain methods
  /// @{

  bool is_bottom() const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [](const Partition& partition) {
                         return partition.memory.is_bottom();
                       });
  }

  bool is_top() const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [](const Partition& partition) {
                         return partition.memory.is_top();
                       });
  }

  void set_to_bottom() override {
    // Remove all partitions except the first one
    this->_partitions.erase(std::next(this->_partitions.begin()),
                            this->_partitions.end());

    // Set the first partition to bottom
    this->_partitions[0].interval.set_to_top();
    this->_partitions[0].memory.set_to_bottom();
  }

  void set_to_top() override {
    // Remove all partitions except the first one
    this->_partitions.erase(std::next(this->_partitions.begin()),
                            this->_partitions.end());

    // Set the first partition to top
    this->_partitions[0].interval.set_to_top();
    this->_partitions[0].memory.set_to_top();
  }

  bool leq(const PartitioningDomain& other) const override {
    if (this->_variable != other._variable) {
      return this->join_partitions().memory.leq(other.join_partitions().memory);
    } else {
      for (auto this_it = this->_partitions.begin(),
                this_et = this->_partitions.end(),
                other_it = other._partitions.begin(),
                other_et = other._partitions.end();
           this_it != this_et;) {
        if (other_it == other_et ||
            this_it->interval.ub() < other_it->interval.lb()) {
          // The partition in `this` does not match any partition in `other`
          if (!this_it->memory.is_bottom()) {
            return false;
          }
          ++this_it;
        } else if (other_it->interval.ub() < this_it->interval.lb()) {
          // The partition in `other` does not match any partition in `this`
          ++other_it;
        } else if (this_it->interval.ub() <= other_it->interval.ub()) {
          // The partition in `this` matches a partition in `other`
          if (!this_it->memory.leq(other_it->memory)) {
            return false;
          }
          ++this_it;
        } else {
          // The partition in `this` overlaps one or more partitions in `other`
          MemoryDomain other_memory = other_it->memory;
          for (auto it = std::next(other_it);
               it != other_et && this_it->interval.ub() >= it->interval.lb();
               ++it) {
            other_memory.join_with(it->memory);
          }
          if (!this_it->memory.leq(other_memory)) {
            return false;
          }
          ++this_it;
        }
      }

      return true;
    }
  }

  bool equals(const PartitioningDomain& other) const override {
    if (this->_variable != other._variable) {
      return this->join_partitions().memory.equals(
          other.join_partitions().memory);
    } else {
      return this->leq(other) && other.leq(*this);
    }
  }

  void join_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      for (const Partition& partition : other._partitions) {
        this->_partitions[0].memory.join_with(partition.memory);
      }
    } else {
      auto this_it = this->_partitions.begin();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      while (other_it != other_et) {
        if (this_it == this->_partitions.end()) {
          // All the remaining partitions in `other` do not match any partition
          // in `this`
          this->_partitions.insert(this->_partitions.end(), other_it, other_et);
          break;
        } else if (this_it->interval.ub() < other_it->interval.lb()) {
          // The partition in `other` is on the right of the partition in `this`
          ++this_it;
        } else if (other_it->interval.ub() < this_it->interval.lb()) {
          // The partition in `other` is on the left of the partition in `this`
          this_it = std::next(this->_partitions.insert(this_it, *other_it));
          ++other_it;
        } else {
          // The partition in `other` overlaps the partition in `this`
          this_it->interval.join_with(other_it->interval);

          // The partition `this` could be bigger now.
          // Join with the following partitions if necessary.
          for (auto it = std::next(this_it);
               it != this->_partitions.end() &&
               this_it->interval.ub() >= it->interval.lb();) {
            this_it->interval.join_with(it->interval);
            this_it->memory.join_with(it->memory);
            it = this->_partitions.erase(it);
            this_it = std::prev(it); // might be invalidated by erase()
          }

          // Join with the partition in `other`
          this_it->memory.join_with(other_it->memory);
          ++other_it;
        }
      }
    }
  }

  void join_loop_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0].memory.join_loop_with(
          other.join_partitions().memory);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.join_loop_with(other_it->memory);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0].interval.join_loop_with(
          other._partitions[0].interval);
      this->_partitions[0].memory.join_loop_with(other._partitions[0].memory);
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0].interval.join_loop_with(other_partition.interval);
      this->_partitions[0].memory.join_loop_with(other_partition.memory);
    }
  }

  void join_iter_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0].memory.join_iter_with(
          other.join_partitions().memory);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.join_iter_with(other_it->memory);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0].interval.join_iter_with(
          other._partitions[0].interval);
      this->_partitions[0].memory.join_iter_with(other._partitions[0].memory);
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0].interval.join_iter_with(other_partition.interval);
      this->_partitions[0].memory.join_iter_with(other_partition.memory);
    }
  }

  void widen_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0].memory.widen_with(other.join_partitions().memory);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.widen_with(other_it->memory);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0].interval.widen_with(other._partitions[0].interval);
      this->_partitions[0].memory.widen_with(other._partitions[0].memory);
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0].interval.widen_with(other_partition.interval);
      this->_partitions[0].memory.widen_with(other_partition.memory);
    }
  }

  void widen_threshold_with(const PartitioningDomain& other,
                            const MachineInt& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0]
          .memory.widen_threshold_with(other.join_partitions().memory,
                                       threshold);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.widen_threshold_with(other_it->memory, threshold);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0]
          .interval.widen_threshold_with(other._partitions[0].interval,
                                         threshold);
      this->_partitions[0]
          .memory.widen_threshold_with(other._partitions[0].memory, threshold);
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0]
          .interval.widen_threshold_with(other_partition.interval, threshold);
      this->_partitions[0].memory.widen_threshold_with(other_partition.memory,
                                                       threshold);
    }
  }

  void meet_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0].memory.meet_with(other.join_partitions().memory);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.meet_with(other_it->memory);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0].interval.meet_with(other._partitions[0].interval);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0].memory.meet_with(other._partitions[0].memory);
      }
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0].interval.meet_with(other_partition.interval);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0].memory.meet_with(other_partition.memory);
      }
    }
  }

  void narrow_with(const PartitioningDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0].memory.narrow_with(other.join_partitions().memory);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.narrow_with(other_it->memory);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0].interval.narrow_with(other._partitions[0].interval);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0].memory.narrow_with(other._partitions[0].memory);
      }
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0].interval.narrow_with(other_partition.interval);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0].memory.narrow_with(other_partition.memory);
      }
    }
  }

  void narrow_threshold_with(const PartitioningDomain& other,
                             const MachineInt& threshold) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_variable != other._variable) {
      this->partitioning_disable();
      this->_partitions[0]
          .memory.narrow_threshold_with(other.join_partitions().memory,
                                        threshold);
    } else if (this->is_same_partitioning(other)) {
      auto this_it = this->_partitions.begin();
      auto this_et = this->_partitions.end();
      auto other_it = other._partitions.begin();
      auto other_et = other._partitions.end();
      for (; this_it != this_et && other_it != other_et;
           ++this_it, ++other_it) {
        ikos_assert(this_it->interval == other_it->interval);
        this_it->memory.narrow_threshold_with(other_it->memory, threshold);
      }
    } else if (other._partitions.size() == 1) {
      this->partitioning_join();
      this->_partitions[0]
          .interval.narrow_threshold_with(other._partitions[0].interval,
                                          threshold);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0]
            .memory.narrow_threshold_with(other._partitions[0].memory,
                                          threshold);
      }
    } else {
      this->partitioning_join();
      Partition other_partition = other.join_partitions();
      this->_partitions[0]
          .interval.narrow_threshold_with(other_partition.interval, threshold);
      if (this->_partitions[0].interval.is_bottom()) {
        this->set_to_bottom();
      } else {
        this->_partitions[0]
            .memory.narrow_threshold_with(other_partition.memory, threshold);
      }
    }
  }

  /// @}
  /// \name Implement uninitialized abstract domain methods
  /// @{

  void uninit_assert_initialized(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.uninit_assert_initialized(x);
    }
  }

  bool uninit_is_initialized(VariableRef x) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.uninit_is_initialized(x);
                       });
  }

  bool uninit_is_uninitialized(VariableRef x) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.uninit_is_uninitialized(x);
                       });
  }

  void uninit_refine(VariableRef x, Uninitialized value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.uninit_refine(x, value);
    }
  }

  Uninitialized uninit_to_uninitialized(VariableRef x) const override {
    auto result = Uninitialized::bottom();
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.uninit_to_uninitialized(x));
    }
    return result;
  }

  /// @}
  /// \name Implement machine integer abstract domain methods
  /// @{

  void int_assign(VariableRef x, const MachineInt& n) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      this->_partitions[0].interval = IntInterval(n);
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_assign(x, n);
    }
  }

  void int_assign_undef(VariableRef x) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      this->_partitions[0].interval.set_to_top();
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_assign_undef(x);
    }
  }

  void int_assign_nondet(VariableRef x) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      this->_partitions[0].interval.set_to_top();
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_assign_nondet(x);
    }
  }

  void int_assign(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_assign(x, y);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_assign(VariableRef x, const IntLinearExpression& e) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_assign(x, e);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_apply(op, x, y);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 VariableRef z) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_apply(op, x, y, z);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 const MachineInt& z) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_apply(op, x, y, z);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 const MachineInt& y,
                 VariableRef z) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_apply(op, x, y, z);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void int_add(IntPredicate pred, VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_add(pred, x, y);
    }
  }

  void int_add(IntPredicate pred, VariableRef x, const MachineInt& y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_add(pred, x, y);
    }
  }

  void int_add(IntPredicate pred, const MachineInt& x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_add(pred, x, y);
    }
  }

  void int_set(VariableRef x, const IntInterval& value) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      if (value.is_bottom()) {
        this->_partitions[0].interval.set_to_top();
      } else {
        this->_partitions[0].interval = value;
      }
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_set(x, value);
    }
  }

  void int_set(VariableRef x, const IntCongruence& value) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      if (value.is_bottom()) {
        this->_partitions[0].interval.set_to_top();
      } else if (value.singleton()) {
        this->_partitions[0].interval = IntInterval(*value.singleton());
      } else {
        this->_partitions[0].interval.set_to_top();
      }
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_set(x, value);
    }
  }

  void int_set(VariableRef x, const IntIntervalCongruence& value) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      if (value.is_bottom()) {
        this->_partitions[0].interval.set_to_top();
      } else {
        this->_partitions[0].interval = value.interval();
      }
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_set(x, value);
    }
  }

  void int_refine(VariableRef x, const IntInterval& value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_refine(x, value);
    }
  }

  void int_refine(VariableRef x, const IntCongruence& value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_refine(x, value);
    }
  }

  void int_refine(VariableRef x, const IntIntervalCongruence& value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.int_refine(x, value);
    }
  }

  void int_forget(VariableRef x) override {
    if (this->_variable && *this->_variable == x) {
      this->partitioning_join();
      this->_partitions[0].interval.set_to_top();
    }

    for (Partition& partition : this->_partitions) {
      partition.memory.int_forget(x);
    }
  }

  IntInterval int_to_interval(VariableRef x) const override {
    auto result = IntInterval::bottom(IntVariableTrait::bit_width(x),
                                      IntVariableTrait::sign(x));
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_interval(x));
    }
    return result;
  }

  IntInterval int_to_interval(const IntLinearExpression& e) const override {
    auto result =
        IntInterval::bottom(e.constant().bit_width(), e.constant().sign());
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_interval(e));
    }
    return result;
  }

  IntCongruence int_to_congruence(VariableRef x) const override {
    auto result = IntCongruence::bottom(IntVariableTrait::bit_width(x),
                                        IntVariableTrait::sign(x));
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_congruence(x));
    }
    return result;
  }

  IntCongruence int_to_congruence(const IntLinearExpression& e) const override {
    auto result =
        IntCongruence::bottom(e.constant().bit_width(), e.constant().sign());
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_congruence(e));
    }
    return result;
  }

  IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const override {
    auto result = IntIntervalCongruence::bottom(IntVariableTrait::bit_width(x),
                                                IntVariableTrait::sign(x));
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_interval_congruence(x));
    }
    return result;
  }

  IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const override {
    auto result = IntIntervalCongruence::bottom(e.constant().bit_width(),
                                                e.constant().sign());
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.int_to_interval_congruence(e));
    }
    return result;
  }

  /// @}
  /// \name Implement non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override {
    ikos_assert(!this->_variable || *this->_variable != x);

    for (Partition& partition : this->_partitions) {
      partition.memory.counter_mark(x);
    }
  }

  void counter_unmark(VariableRef x) override {
    ikos_assert(!this->_variable || *this->_variable != x);

    for (Partition& partition : this->_partitions) {
      partition.memory.counter_unmark(x);
    }
  }

  void counter_init(VariableRef x, const MachineInt& c) override {
    ikos_assert(!this->_variable || *this->_variable != x);

    for (Partition& partition : this->_partitions) {
      partition.memory.counter_init(x, c);
    }
  }

  void counter_incr(VariableRef x, const MachineInt& k) override {
    ikos_assert(!this->_variable || *this->_variable != x);

    for (Partition& partition : this->_partitions) {
      partition.memory.counter_incr(x, k);
    }
  }

  void counter_forget(VariableRef x) override {
    ikos_assert(!this->_variable || *this->_variable != x);

    for (Partition& partition : this->_partitions) {
      partition.memory.counter_forget(x);
    }
  }

  /// @}
  /// \name Implement floating point abstract domain methods
  /// @{

  void float_assign_undef(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.float_assign_undef(x);
    }
  }

  void float_assign_nondet(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.float_assign_nondet(x);
    }
  }

  void float_assign(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.float_assign(x, y);
    }
  }

  void float_forget(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.float_forget(x);
    }
  }

  /// @}
  /// \name Implement nullity abstract domain methods
  /// @{

  void nullity_assert_null(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.nullity_assert_null(p);
    }
  }

  void nullity_assert_non_null(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.nullity_assert_non_null(p);
    }
  }

  bool nullity_is_null(VariableRef p) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.nullity_is_null(p);
                       });
  }

  bool nullity_is_non_null(VariableRef p) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.nullity_is_non_null(p);
                       });
  }

  void nullity_set(VariableRef p, Nullity value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.nullity_set(p, value);
    }
  }

  void nullity_refine(VariableRef p, Nullity value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.nullity_refine(p, value);
    }
  }

  Nullity nullity_to_nullity(VariableRef p) const override {
    auto result = Nullity::bottom();
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.nullity_to_nullity(p));
    }
    return result;
  }

  /// @}
  /// \name Implement pointer abstract domain methods
  /// @{

  void pointer_assign(VariableRef p,
                      MemoryLocationRef addr,
                      Nullity nullity) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign(p, addr, nullity);
    }
  }

  void pointer_assign_null(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign_null(p);
    }
  }

  void pointer_assign_undef(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign_undef(p);
    }
  }

  void pointer_assign_nondet(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign_nondet(p);
    }
  }

  void pointer_assign(VariableRef p, VariableRef q) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign(p, q);
    }
  }

  void pointer_assign(VariableRef p, VariableRef q, VariableRef o) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign(p, q, o);
    }
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const MachineInt& o) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign(p, q, o);
    }
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const IntLinearExpression& o) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_assign(p, q, o);
    }
  }

  void pointer_add(PointerPredicate pred,
                   VariableRef p,
                   VariableRef q) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_add(pred, p, q);
    }
  }

  void pointer_refine(VariableRef p, const PointsToSetT& addrs) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_refine(p, addrs);
    }
  }

  void pointer_refine(VariableRef p,
                      const PointsToSetT& addrs,
                      const IntInterval& offset) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_refine(p, addrs, offset);
    }
  }

  void pointer_refine(VariableRef p, const PointerAbsValueT& value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_refine(p, value);
    }
  }

  void pointer_refine(VariableRef p, const PointerSetT& set) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_refine(p, set);
    }
  }

  void pointer_offset_to_int(VariableRef x, VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_offset_to_int(x, p);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  IntInterval pointer_offset_to_interval(VariableRef p) const override {
    auto result = this->_partitions[0].memory.pointer_offset_to_interval(p);
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      result.join_with(it->memory.pointer_offset_to_interval(p));
    }
    return result;
  }

  IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
    auto result = this->_partitions[0].memory.pointer_offset_to_congruence(p);
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      result.join_with(it->memory.pointer_offset_to_congruence(p));
    }
    return result;
  }

  IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const override {
    auto result =
        this->_partitions[0].memory.pointer_offset_to_interval_congruence(p);
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      result.join_with(it->memory.pointer_offset_to_interval_congruence(p));
    }
    return result;
  }

  PointsToSetT pointer_to_points_to(VariableRef p) const override {
    auto result = this->_partitions[0].memory.pointer_to_points_to(p);
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      result.join_with(it->memory.pointer_to_points_to(p));
    }
    return result;
  }

  PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
    auto result = this->_partitions[0].memory.pointer_to_pointer(p);
    for (auto it = std::next(this->_partitions.begin()),
              et = this->_partitions.end();
         it != et;
         ++it) {
      result.join_with(it->memory.pointer_to_pointer(p));
    }
    return result;
  }

  void pointer_forget_offset(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_forget_offset(p);
    }
  }

  void pointer_forget(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.pointer_forget(p);
    }
  }

  /// @}
  /// \name Implement dynamically typed variables abstract domain methods
  /// @{

  void dynamic_assign(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_assign(x, y);
    }
  }

  void dynamic_write_undef(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_undef(x);
    }
  }

  void dynamic_write_nondet(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_nondet(x);
    }
  }

  void dynamic_write_int(VariableRef x, const MachineInt& n) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_int(x, n);
    }
  }

  void dynamic_write_nondet_int(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_nondet_int(x);
    }
  }

  void dynamic_write_int(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_int(x, y);
    }
  }

  void dynamic_write_nondet_float(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_nondet_float(x);
    }
  }

  void dynamic_write_null(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_null(x);
    }
  }

  void dynamic_write_pointer(VariableRef x,
                             MemoryLocationRef addr,
                             Nullity nullity) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_pointer(x, addr, nullity);
    }
  }

  void dynamic_write_pointer(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_write_pointer(x, y);
    }
  }

  void dynamic_read_int(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_read_int(x, y);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void dynamic_read_pointer(VariableRef x, VariableRef y) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_read_pointer(x, y);
    }
  }

  bool dynamic_is_zero(VariableRef x) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.dynamic_is_zero(x);
                       });
  }

  bool dynamic_is_null(VariableRef x) const override {
    return std::all_of(this->_partitions.begin(),
                       this->_partitions.end(),
                       [=](const Partition& partition) {
                         return partition.memory.dynamic_is_null(x);
                       });
  }

  void dynamic_forget(VariableRef x) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.dynamic_forget(x);
    }
  }

  /// @}
  /// \name Implement scalar abstract domain methods
  /// @{

  void scalar_assign_undef(VariableRef x) override {
    if (ScalarVariableTrait::is_int(x)) {
      this->int_assign_undef(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_assign_undef(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_assign_undef(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_write_undef(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  void scalar_assign_nondet(VariableRef x) override {
    if (ScalarVariableTrait::is_int(x)) {
      this->int_assign_nondet(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_assign_nondet(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_assign_nondet(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_write_nondet(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  void scalar_pointer_to_int(VariableRef x,
                             VariableRef p,
                             MemoryLocationRef absolute_zero) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.scalar_pointer_to_int(x, p, absolute_zero);
    }

    if (this->_variable && *this->_variable == x) {
      this->update_partitions();
    }
  }

  void scalar_int_to_pointer(VariableRef p,
                             VariableRef x,
                             MemoryLocationRef absolute_zero) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.scalar_int_to_pointer(p, x, absolute_zero);
    }
  }

  void scalar_forget(VariableRef x) override {
    if (ScalarVariableTrait::is_int(x)) {
      this->int_forget(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_forget(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_forget(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_forget(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  /// @}
  /// \name Implement memory abstract domain methods
  /// @{

  void mem_write(VariableRef p,
                 const LiteralT& v,
                 const MachineInt& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_write(p, v, size);
    }
  }

  void mem_read(const LiteralT& x,
                VariableRef p,
                const MachineInt& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_read(x, p, size);
    }

    if (this->_variable && x.is_machine_int_var() &&
        *this->_variable == x.var()) {
      this->update_partitions();
    }
  }

  void mem_copy(VariableRef dest,
                VariableRef src,
                const LiteralT& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_copy(dest, src, size);
    }
  }

  void mem_set(VariableRef dest,
               const LiteralT& value,
               const LiteralT& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_set(dest, value, size);
    }
  }

  void mem_forget_all() override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget_all();
    }
  }

  void mem_forget(MemoryLocationRef addr) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget(addr);
    }
  }

  void mem_forget(MemoryLocationRef addr,
                  const IntInterval& offset,
                  const MachineInt& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget(addr, offset, size);
    }
  }

  void mem_forget(MemoryLocationRef addr, const IntInterval& range) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget(addr, range);
    }
  }

  void mem_forget_reachable(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget_reachable(p);
    }
  }

  void mem_forget_reachable(VariableRef p, const MachineInt& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_forget_reachable(p, size);
    }
  }

  void mem_abstract_reachable(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_abstract_reachable(p);
    }
  }

  void mem_abstract_reachable(VariableRef p, const MachineInt& size) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_abstract_reachable(p, size);
    }
  }

  void mem_zero_reachable(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_zero_reachable(p);
    }
  }

  void mem_uninitialize_reachable(VariableRef p) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.mem_uninitialize_reachable(p);
    }
  }

  /// @}
  /// \name Lifetime abstract domain methods
  /// @{

  void lifetime_assign_allocated(MemoryLocationRef m) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_assign_allocated(m);
    }
  }

  void lifetime_assign_deallocated(MemoryLocationRef m) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_assign_deallocated(m);
    }
  }

  void lifetime_assert_allocated(MemoryLocationRef m) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_assert_allocated(m);
    }
  }

  void lifetime_assert_deallocated(MemoryLocationRef m) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_assert_deallocated(m);
    }
  }

  void lifetime_set(MemoryLocationRef m, Lifetime value) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_set(m, value);
    }
  }

  void lifetime_forget(MemoryLocationRef m) override {
    for (Partition& partition : this->_partitions) {
      partition.memory.lifetime_forget(m);
    }
  }

  Lifetime lifetime_to_lifetime(MemoryLocationRef m) const override {
    auto result = Lifetime::bottom();
    for (const Partition& partition : this->_partitions) {
      result.join_with(partition.memory.lifetime_to_lifetime(m));
    }
    return result;
  }

  /// @}

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (!this->_variable) {
      ikos_assert(this->_partitions.size() == 1);

      this->_partitions[0].memory.dump(o);
    } else {
      ikos_assert(this->_partitions.size() >= 1);

      o << "{";
      for (auto it = this->_partitions.begin(), et = this->_partitions.end();
           it != et;) {
        core::DumpableTraits< VariableRef >::dump(o, *this->_variable);
        o << " ∈ ";
        it->interval.dump(o);
        o << " -> ";
        it->memory.dump(o);
        ++it;
        if (it != et) {
          o << ", ";
        }
      }
      o << "}";
    }
  }

  static std::string name() {
    return "partitioning domain using " + MemoryDomain::name();
  }

}; // end class PartitioningDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
