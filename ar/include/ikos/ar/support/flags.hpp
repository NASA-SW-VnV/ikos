/*******************************************************************************
 *
 * \file
 * \brief Flags class definitions
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

#pragma once

namespace ikos {
namespace ar {

/// \brief Type-safe way of storing OR-combinations of enum values
template < typename Enum >
class Flags {
private:
  static_assert(sizeof(Enum) <= sizeof(unsigned), "enum not supported");

public:
  using EnumType = Enum;

private:
  // Flags value, as an unsigned integer
  unsigned _v = 0;

private:
  explicit Flags(unsigned v) : _v(v) {}

public:
  Flags() = default;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  Flags(Enum v) : _v(v) {}

  Flags(const Flags&) noexcept = default;

  Flags(Flags&&) noexcept = default;

  Flags& operator=(const Flags&) noexcept = default;

  Flags& operator=(Flags&&) noexcept = default;

  ~Flags() = default;

  /// \name Bitwise assignment operators
  /// @{

  Flags& operator&=(Enum f) {
    _v &= unsigned(f);
    return *this;
  }

  Flags& operator&=(Flags f) {
    _v &= f._v;
    return *this;
  }

  Flags& operator|=(Enum f) {
    _v |= unsigned(f);
    return *this;
  }

  Flags& operator|=(Flags f) {
    _v |= f._v;
    return *this;
  }

  Flags& operator^=(Enum f) {
    _v ^= unsigned(f);
    return *this;
  }

  Flags& operator^=(Flags f) {
    _v ^= f._v;
    return *this;
  }

  /// @}
  /// \name Bitwise operators
  /// @{

  Flags operator&(Enum f) const { return Flags(_v & unsigned(f)); }
  Flags operator&(Flags f) const { return Flags(_v & f._v); }

  Flags operator|(Enum f) const { return Flags(_v | unsigned(f)); }
  Flags operator|(Flags f) const { return Flags(_v | f._v); }

  Flags operator^(Enum f) const { return Flags(_v ^ unsigned(f)); }
  Flags operator^(Flags f) const { return Flags(_v ^ f._v); }

  Flags operator~() const { return Flags(~_v); }

  bool operator!() const { return _v == 0; }

  /// @}

  /// \brief Check whether the given flag is set to 1
  bool test(Enum f) const {
    if (f == 0) {
      return _v == 0;
    } else {
      return (_v & unsigned(f)) == unsigned(f);
    }
  }

  /// \brief Set the given flag to 1 if `on` is true, 0 otherwise
  Flags& set(Enum f, bool on = true) {
    if (on) {
      _v |= unsigned(f);
    } else {
      _v &= ~unsigned(f);
    }
    return *this;
  }
};

/// \macros IKOS_DECLARE_OPERATORS_FOR_FLAGS
///
/// Define operator|(EnumType, EnumType)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define IKOS_DECLARE_OPERATORS_FOR_FLAGS(T)            \
  inline T operator|(T::EnumType f1, T::EnumType f2) { \
    return T(f1) | f2;                                 \
  }

} // end namespace ar
} // end namespace ikos
