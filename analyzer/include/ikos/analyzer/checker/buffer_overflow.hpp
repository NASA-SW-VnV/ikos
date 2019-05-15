/*******************************************************************************
 *
 * \file
 * \brief Buffer overflow checker
 *
 * Author: Maxime Arthaud
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

#include <utility>

#include <llvm/ADT/SmallVector.h>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Buffer overflow checker
class BufferOverflowChecker final : public Checker {
private:
  using PointsToSet = core::PointsToSet< MemoryLocation* >;
  using IntPredicate = core::machine_int::Predicate;
  using IntUnaryOperator = core::machine_int::UnaryOperator;
  using IntBinaryOperator = core::machine_int::BinaryOperator;
  using IntInterval = core::machine_int::Interval;
  using IntVariable = core::VariableExpression< MachineInt, Variable* >;
  using IntLinearExpression = core::LinearExpression< MachineInt, Variable* >;

private:
  //// \brief The AR context
  ar::Context& _ar_context;

  /// \brief The data layout
  const ar::DataLayout& _data_layout;

  /// \brief The unsigned integer type with the bit-width of a pointer
  ar::IntegerType* _size_type;

  /// \brief The integer constant 0 of type size_t
  ar::IntegerConstant* _size_zero;

  /// \brief The integer constant 1 of type size_t
  ar::IntegerConstant* _size_one;

public:
  enum class BufferOverflowCheckKind {
    /// \brief Check for a memory access on a function memory location
    Function = 0,

    /// \brief Check for a memory access on a deallocated dynamic allocated
    /// memory
    UseAfterFree = 1,

    /// \brief Check for a memory access on a dangling stack pointer
    UseAfterReturn = 2,

    /// \brief Check for a memory access on a hardware address
    HardwareAddresses = 3,

    // \brief Check for an out of bound memory access
    OutOfBound = 4,
  };

public:
  /// \brief Constructor
  explicit BufferOverflowChecker(Context& ctx);

  /// \brief Get the checker name
  CheckerName name() const override;

  /// \brief Get the checker description
  const char* description() const override;

  /// \brief Check a statement
  void check(ar::Statement* stmt,
             const value::AbstractDomain& inv,
             CallContext* call_context) override;

private:
  /// \brief Check result
  struct CheckResult {
    CheckKind kind;
    Result result;
    llvm::SmallVector< ar::Value*, 2 > operands;
    JsonDict info;
  };

  /// \brief Memory location check result
  struct MemoryLocationCheckResult {
    BufferOverflowCheckKind kind;
    Result result;
  };

  /// \brief Check a function call
  std::vector< CheckResult > check_call(ar::CallBase* call,
                                        const value::AbstractDomain& inv);

  /// \brief Check an intrinsic function call
  std::vector< CheckResult > check_intrinsic_call(
      ar::CallBase* call, ar::Function* fun, const value::AbstractDomain& inv);

  /// \brief Check a memory access (read/write) for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// The method checks that the memory access to
  /// [offset, offset + access_size - 1] is valid.
  ///
  /// It checks the following property:
  ///   addrs_set(ptr) != TOP &&
  ///   ∀a ∈ addrs_set(ptr), ∀o ∈ offset, o + access_size <= a.size
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param access_size The access size operand (in bytes)
  /// \param if_null Result if the pointer is null
  /// \param inv The invariant
  CheckResult check_mem_access(ar::Statement* stmt,
                               ar::Value* pointer,
                               ar::Value* access_size,
                               Result if_null,
                               value::AbstractDomain inv);

  /// \brief Check a memory access (read/write) at the given memory location
  ///
  /// The method is called by check_mem_access, and performs the analysis
  /// of the memory location access.
  ///
  /// \returns A pair which contains the result and the kind of the check
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param access_size The access size operand (in bytes)
  /// \param inv The invariant
  /// \param addr The memory location
  /// \param size_var The allocation size variable
  /// \param offset_var The pointer offset variable
  /// \param offset_plus_size The shadow variable equal to offset + access size
  /// \param offset_intv The pointer offset interval
  /// \param block_info JSON dictionary to add extra information
  MemoryLocationCheckResult check_memory_location_access(
      ar::Statement* stmt,
      ar::Value* pointer,
      ar::Value* access_size,
      const value::AbstractDomain& inv,
      MemoryLocation* addr,
      AllocSizeVariable* size_var,
      Variable* offset_var,
      Variable* offset_plus_size,
      const IntInterval& offset_intv,
      JsonDict& block_info);

  /// \brief Check a string access (read/write) for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param stmt The statement
  /// \param pointer The pointer string operand
  /// \param if_null Result if the pointer is null
  /// \param inv The invariant
  CheckResult check_string_access(ar::Statement* stmt,
                                  ar::Value* pointer,
                                  Result if_null,
                                  const value::AbstractDomain& inv);

  /// \brief Check a string access (read/write) for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param stmt The statement
  /// \param pointer The pointer string operand
  /// \param max_access_size The maximum access size, in bytes
  /// \param if_null Result if the pointer is null
  /// \param inv The invariant
  CheckResult check_string_access(ar::Statement* stmt,
                                  ar::Value* pointer,
                                  ar::Value* max_access_size,
                                  Result if_null,
                                  const value::AbstractDomain& inv);

  /// \brief Check a `va_list` access (read/write) for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param inv The invariant
  CheckResult check_va_list_access(ar::Statement* stmt,
                                   ar::Value* pointer,
                                   const value::AbstractDomain& inv);

  /// \brief Check a call to realloc for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param call The call statement
  /// \param pointer The pointer operand
  /// \param inv The invariant
  CheckResult check_realloc(ar::CallBase* call,
                            ar::Value* pointer,
                            const value::AbstractDomain& inv);

  /// \brief Check a `FILE*` access (read/write) for buffer overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param if_null Result if the pointer is null
  /// \param inv The invariant
  CheckResult check_file_access(ar::Statement* stmt,
                                ar::Value* pointer,
                                Result if_null,
                                const value::AbstractDomain& inv);

  /// \brief Return the store size for the given type, as an integer constant
  ar::IntegerConstant* store_size(ar::Type*) const;

  /// \brief Initialize global variable pointers and function pointers
  void init_global_ptr(value::AbstractDomain& inv, ar::Value* value) const;

  /// \brief Initialize global variable sizes and function sizes
  void init_global_alloc_size(value::AbstractDomain& inv,
                              MemoryLocation* addr,
                              AllocSizeVariable* size_var) const;

  /// \brief Check whether a memory access is an array access
  ///
  /// \returns the size of an array element
  boost::optional< MachineInt > is_array_access(
      ar::Statement* stmt,
      const value::AbstractDomain& inv,
      const IntInterval& offset_intv,
      const PointsToSet& addrs) const;

  /// \brief Display a memory access check, if requested
  llvm::Optional< LogMessage > display_mem_access_check(
      Result result, ar::Statement* stmt) const;

  /// \brief Display a memory access check, if requested
  llvm::Optional< LogMessage > display_mem_access_check(
      Result result,
      ar::Statement* stmt,
      ar::Value* pointer,
      ar::Value* access_size) const;

  /// \brief Display a memory access check, if requested
  llvm::Optional< LogMessage > display_mem_access_check(
      Result result,
      ar::Statement* stmt,
      ar::Value* pointer,
      ar::Value* access_size,
      MemoryLocation* addr) const;

}; // end class BufferOverflowChecker

} // end namespace analyzer
} // end namespace ikos
