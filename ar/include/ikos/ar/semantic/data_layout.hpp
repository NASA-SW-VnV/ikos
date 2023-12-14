/*******************************************************************************
 *
 * \file
 * \brief Data Layout definition
 *
 * Define size and alignment of AR data types.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2023 United States Government as represented by the
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

#include <memory>
#include <vector>

#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/number.hpp>

namespace ikos {
namespace ar {

/// \brief Endianness
enum Endianness { LittleEndian, BigEndian };

// forward declaration
class Type;

/// \brief Return the next integer that is greater than or equal to \p value
/// and is a multiple of \p align.
///
/// \p align must be non-zero.
///
/// Examples:
/// \code
///   align_to(5, 8) = 8
///   align_to(17, 8) = 24
///   align_to(321, 255) = 510
/// \endcode
inline ZNumber align_to(const ZNumber& value, const ZNumber& align) {
  ikos_assert_msg(align != 0, "alignment cannot be 0");
  return (value + align - 1) / align * align;
}

/// \brief Information about a data type
struct DataLayoutInfo {
  /// \brief Bit width
  uint64_t bit_width;

  /// \brief ABI alignment, in bytes
  uint64_t abi_alignment;

  /// \brief Preferred alignment, in bytes
  uint64_t pref_alignment;

  /// \brief Default constructor
  DataLayoutInfo(uint64_t bit_width_,
                 uint64_t abi_alignment_,
                 uint64_t pref_alignment_)
      : bit_width(bit_width_),
        abi_alignment(abi_alignment_),
        pref_alignment(pref_alignment_) {}

}; // end struct DataLayoutInfo

/// \brief Target data layout
///
/// This class holds information about data type sizes and alignments.
class DataLayout {
public:
  /// \brief Endianness (LittleEndian or BigEndian)
  Endianness endianness;

  /// \brief Information about pointers
  DataLayoutInfo pointers;

  /// \brief Information about integer types (ordered by bit-width)
  std::vector< DataLayoutInfo > integers;

  /// \brief Information about floating point types (ordered by bit-width)
  std::vector< DataLayoutInfo > floats;

private:
  /// \brief Default constructor
  DataLayout(Endianness endianness, DataLayoutInfo pointers);

public:
  /// \brief No copy constructor
  DataLayout(const DataLayout&) = delete;

  /// \brief No move constructor
  DataLayout(DataLayout&&) = delete;

  /// \brief No copy assignment operator
  DataLayout& operator=(const DataLayout&) = delete;

  /// \brief No move assignment operator
  DataLayout& operator=(DataLayout&&) = delete;

  /// \brief Destructor
  ~DataLayout();

  /// \name Constructors
  /// @{

  /// \brief Static constructor
  static std::unique_ptr< DataLayout > create(Endianness endianness,
                                              DataLayoutInfo pointers);

  /// @}
  /// \name Endianness
  /// @{

  bool is_little_endian() const { return this->endianness == LittleEndian; }

  bool is_big_endian() const { return this->endianness == BigEndian; }

  /// @}
  /// \name Type alignments
  /// @{

  /// \brief Set the ABI and preferred alignment for integers of the given
  /// bit-with.
  void set_integer_alignment(DataLayoutInfo info);

  /// \brief Set the ABI and preferred alignment for floats of the given
  /// bit-with.
  void set_float_alignment(DataLayoutInfo info);

  /// \brief Return the minimum ABI-required alignment for the given type,
  /// in bytes.
  ZNumber abi_alignment(Type*) const;

  /// \brief Return the preferred alignment for the given type,
  /// in bytes.
  ZNumber pref_alignment(Type*) const;

  /// @}
  /// \name Type sizes
  /// @{

  /// \brief Return the number of bits necessary to hold the given type.
  ///
  /// For example, returns 36 for i36 and 80 for x86_fp80.
  ///
  /// Examples:
  ///
  /// Type        size_in_bits  store_size_in_bits  alloc_size_in_bits
  /// ----        ------------  ------------------  ------------------
  ///  i1                    1                   8                   8
  ///  i8                    8                   8                   8
  ///  i19                  19                  24                  32
  ///  i32                  32                  32                  32
  ///  i100                100                 104                 128
  ///  i128                128                 128                 128
  ///  Float                32                  32                  32
  ///  Double               64                  64                  64
  ///  X86_FP80             80                  80                  96
  ZNumber size_in_bits(Type*) const;

  /// \brief Return the maximum number of bytes that may be overwritten by
  /// storing the specified type.
  ///
  /// For example, returns 5 for i36 and 10 for x86_fp80.
  ZNumber store_size_in_bytes(Type* t) const {
    return (this->size_in_bits(t) + 7) / 8;
  }

  /// \brief Return the maximum number of bits that may be overwritten by
  /// storing the specified type
  //
  /// Always a multiple of 8.
  ///
  /// For example, returns 40 for i36 and 80 for x86_fp80.
  ZNumber store_size_in_bits(Type* t) const {
    return this->store_size_in_bytes(t) * 8;
  }

  /// \brief Return the offset in bytes between successive objects of the
  /// specified type, including alignment padding.
  ///
  /// This is the amount of space that Allocate reserves for this type.
  /// For example, returns 12 or 16 for x86_fp80, depending on alignment.
  ZNumber alloc_size_in_bytes(Type* t) const {
    return align_to(this->store_size_in_bytes(t), this->abi_alignment(t));
  }

  /// \brief Return the offset in bits between successive objects of the
  /// specified type, including alignment padding.
  ///
  /// Always a multiple of 8.
  ///
  /// This is the amount of space that Allocate reserves for this type.
  /// For example, returns 96 or 128 for x86_fp80, depending on alignment.
  ZNumber alloc_size_in_bits(Type* t) const {
    return this->alloc_size_in_bytes(t) * 8;
  }

  /// @}

}; // end class DataLayout

} // end namespace ar
} // end namespace ikos
