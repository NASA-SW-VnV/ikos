/*******************************************************************************
 *
 * \file
 * \brief Implementation of DataLayout
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/ar/semantic/data_layout.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/type_visitor.hpp>

namespace ikos {
namespace ar {

DataLayout::DataLayout(Endianness endianness_, DataLayoutInfo pointers_)
    : endianness(endianness_), pointers(pointers_) {
  // Default alignments for integers
  integers.emplace_back(DataLayoutInfo(1, 1, 1));
  integers.emplace_back(DataLayoutInfo(8, 1, 1));
  integers.emplace_back(DataLayoutInfo(16, 2, 2));
  integers.emplace_back(DataLayoutInfo(32, 4, 4));
  integers.emplace_back(DataLayoutInfo(64, 8, 8));

  // Default alignments for floats
  floats.emplace_back(DataLayoutInfo(16, 2, 2));
  floats.emplace_back(DataLayoutInfo(32, 4, 4));
  floats.emplace_back(DataLayoutInfo(64, 8, 8));
  floats.emplace_back(DataLayoutInfo(128, 16, 16));
}

DataLayout::~DataLayout() = default;

std::unique_ptr< DataLayout > DataLayout::create(Endianness endianness,
                                                 DataLayoutInfo pointers) {
  return std::unique_ptr< DataLayout >(new DataLayout(endianness, pointers));
}

/// \brief Return the next power of two that is strictly greater than n
static ZNumber next_power_of_2(const ZNumber& n) {
  ZNumber r(1);
  while (r <= n) {
    r <<= 1;
  }
  return r;
}

/// \brief Return the power of 2 which is greater than or equal to the given
/// value
static ZNumber power_of_2_ceil(const ZNumber& n) {
  if (n <= 2) {
    return n;
  } else {
    return next_power_of_2(n - 1);
  }
}

/// \brief Add a DataLayoutInfo into a vector of DataLayoutInfo
static void update_data_layout_infos(std::vector< DataLayoutInfo >& v,
                                     DataLayoutInfo info) {
  auto it = std::lower_bound(v.begin(),
                             v.end(),
                             info,
                             [](const DataLayoutInfo& lhs,
                                const DataLayoutInfo& rhs) {
                               return lhs.bit_width < rhs.bit_width;
                             });

  if (it != v.end() && it->bit_width == info.bit_width) {
    // Update an existing entry
    it->abi_alignment = info.abi_alignment;
    it->pref_alignment = info.pref_alignment;
  } else {
    // Otherwise, create it
    v.insert(it, info);
  }
}

/// \brief Find the best alignment for a bit width
static uint64_t find_alignment_info(const std::vector< DataLayoutInfo >& v,
                                    uint64_t bit_width,
                                    bool abi) {
  ikos_assert(!v.empty());

  auto it = std::lower_bound(v.begin(),
                             v.end(),
                             DataLayoutInfo(bit_width, 0, 0),
                             [](const DataLayoutInfo& lhs,
                                const DataLayoutInfo& rhs) {
                               return lhs.bit_width < rhs.bit_width;
                             });

  if (it == v.end()) {
    // If we didn't have a larger value, use the largest value we have
    it = std::prev(v.end());
  }
  // Else, use the exact match or first larger value

  if (abi) {
    return it->abi_alignment;
  } else {
    return it->pref_alignment;
  }
}

void DataLayout::set_integer_alignment(DataLayoutInfo info) {
  update_data_layout_infos(this->integers, info);
}

void DataLayout::set_float_alignment(DataLayoutInfo info) {
  update_data_layout_infos(this->floats, info);
}

namespace {

/// \brief Compute the alignment (ABI or Preferred) for a given type
class TypeAlignmentVisitor {
public:
  using ResultType = ZNumber;

public:
  // Data layout
  const DataLayout& data_layout;

  // ABI or Preferred alignment (true = abi, false = preferred)
  bool abi;

public:
  /// \brief Constructor
  TypeAlignmentVisitor(const DataLayout& data_layout_, bool abi_)
      : data_layout(data_layout_), abi(abi_) {}

public:
  ZNumber operator()(VoidType* /*type*/) const {
    ikos_unreachable("no alignment");
  }

  ZNumber operator()(IntegerType* type) const {
    return ZNumber(
        find_alignment_info(data_layout.integers, type->bit_width(), abi));
  }

  ZNumber operator()(FloatType* type) const {
    return ZNumber(
        find_alignment_info(data_layout.floats, type->bit_width(), abi));
  }

  ZNumber operator()(PointerType* /*type*/) const {
    if (abi) {
      return ZNumber(data_layout.pointers.abi_alignment);
    } else {
      return ZNumber(data_layout.pointers.pref_alignment);
    }
  }

  ZNumber operator()(StructType* type) const {
    if (type->packed()) {
      // Packed structures have alignment of 1 byte
      return ZNumber(1);
    }

    // Find the max of the alignment of all fields
    ZNumber alignment(0);

    for (auto it = type->field_begin(), et = type->field_end(); it != et;
         ++it) {
      alignment = std::max(alignment, apply_visitor(*this, it->type));
    }

    if (alignment == 0) {
      // Empty structures have alignment of 1 byte
      alignment = 1;
    }

    return alignment;
  }

  ZNumber operator()(ArrayType* t) const {
    return apply_visitor(*this, t->element_type());
  }

  ZNumber operator()(VectorType* t) const {
    ZNumber align = data_layout.alloc_size_in_bytes(t->element_type());
    align *= t->num_elements();
    align = power_of_2_ceil(align);
    return align;
  }

  ZNumber operator()(OpaqueType* /*t*/) const {
    ikos_unreachable("no alignment");
  }

  ZNumber operator()(FunctionType* /*t*/) const {
    ikos_unreachable("no alignment");
  }

}; // end class TypeAlignmentVisitor

} // end anonymous namespace

ZNumber DataLayout::abi_alignment(Type* type) const {
  return apply_visitor(TypeAlignmentVisitor(*this, true), type);
}

ZNumber DataLayout::pref_alignment(Type* type) const {
  return apply_visitor(TypeAlignmentVisitor(*this, false), type);
}

namespace {

/// \brief Compute the size (in bits) for a given type
class TypeSizeVisitor {
public:
  using ResultType = ZNumber;

public:
  // Data layout
  const DataLayout& data_layout;

public:
  /// \brief Constructor
  explicit TypeSizeVisitor(const DataLayout& data_layout_)
      : data_layout(data_layout_) {}

public:
  ZNumber operator()(VoidType*) const { return ZNumber(0); }

  ZNumber operator()(IntegerType* type) const {
    return ZNumber(type->bit_width());
  }

  ZNumber operator()(FloatType* type) const {
    return ZNumber(type->bit_width());
  }

  ZNumber operator()(PointerType*) const {
    return ZNumber(data_layout.pointers.bit_width);
  }

  ZNumber operator()(StructType* type) const {
    if (type->empty()) {
      return ZNumber(0);
    }

    // Get the last field
    auto it = type->field_rbegin();

    // Offset of last field (in bytes)
    ZNumber size = it->offset;

    // Size of last field type (in bytes)
    size += data_layout.alloc_size_in_bytes(it->type);

    // Add padding to the end of the struct so that it could be put in an array
    // and all array elements would be aligned correctly.
    size = align_to(size, data_layout.abi_alignment(type));

    // In bits
    return size * 8;
  }

  ZNumber operator()(ArrayType* t) const {
    return t->num_elements() *
           data_layout.alloc_size_in_bits(t->element_type());
  }

  ZNumber operator()(VectorType* t) const {
    return t->num_elements() * apply_visitor(*this, t->element_type());
  }

  ZNumber operator()(OpaqueType* /*t*/) const { return ZNumber(0); }

  ZNumber operator()(FunctionType* /*t*/) const { return ZNumber(0); }

}; // end class TypeSizeVisitor

} // end anonymous namespace

ZNumber DataLayout::size_in_bits(Type* t) const {
  return apply_visitor(TypeSizeVisitor(*this), t);
}

} // end namespace ar
} // end namespace ikos
