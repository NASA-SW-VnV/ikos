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
 * Copyright (c) 2011-2018 United States Government as represented by the
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
  ar::IntegerType* _offset_type;

  /// \brief The integer constant 1
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

    /// \brief Check for a memory access on an invalid hardware address
    HardwareAddresses = 3,

    // \brief Check for a out of bound memory access
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

  /// \brief Check a memory access (read/write) for buffer overflow
  ///
  /// The method checks that the memory access is valid and writes the result
  /// in the database.
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param access_size The read/written size (in bytes)
  /// \param inv The invariant
  /// \param call_context The calling context
  void check_mem_access(ar::Statement* stmt,
                        ar::Value* pointer,
                        ar::Value* access_size,
                        const value::AbstractDomain& inv,
                        CallContext* call_context);

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
  /// \param access_size The read/written size (in bytes)
  /// \param inv The invariant
  CheckResult check_mem_access(ar::Statement* stmt,
                               ar::Value* pointer,
                               ar::Value* access_size,
                               value::AbstractDomain inv);

  /// \brief Check a memory access (read/write)
  ///
  /// The method is called by check_mem_access, and performs the analysis
  /// of the memory location access.
  ///
  /// \returns A pair which contains the result and the kind of the check
  ///
  /// \param stmt The statement
  /// \param pointer The
  /// \param access_size The read/written size (in bytes)
  /// \param inv The invariant
  /// \param addr The memory location
  /// \param size_var Size of the variable
  /// \param offset_var The pointer offset
  /// \param offset_plus_size Shadow variable, = offset + access size
  /// \param offset_intv Offset int interval
  /// \param block_info Json dictionnary for adding extra information
  std::pair< Result, BufferOverflowCheckKind > check_memory_location_access(
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

  /// \brief Check a string copy for overflow
  ///
  /// The method checks that the memory access is valid and writes the result
  /// in the database.
  ///
  /// \param stmt The statement
  /// \param dest_op The destination pointer operand
  /// \param src_op The source pointer operand
  /// \param inv The invariant
  /// \param call_context The calling context
  void check_strcpy(ar::Statement* stmt,
                    ar::Value* dest_op,
                    ar::Value* src_op,
                    const value::AbstractDomain& inv,
                    CallContext* call_context);

  /// \brief Check a string copy for overflow
  ///
  /// \returns The analysis check result (Ok, Warning, Error, Unreachable)
  ///
  /// \param stmt The statement
  /// \param dest_op The destination pointer operand
  /// \param src_op The source pointer operand
  /// \param inv The invariant
  CheckResult check_strcpy(ar::Statement* stmt,
                           ar::Value* dest_op,
                           ar::Value* src_op,
                           value::AbstractDomain inv);

  /// \brief Return the store size for the given type, as an integer constant
  ar::IntegerConstant* store_size(ar::Type*);

  /// \brief Initialize global variable pointers and function pointers
  void init_global_ptr(ar::Value* value, value::AbstractDomain& inv);

  /// \brief Initialize global variable sizes and function sizes
  void init_global_alloc_size(MemoryLocation* addr,
                              AllocSizeVariable* size_var,
                              value::AbstractDomain& inv);

  /// \brief Check whether a memory access is an array access
  ///
  /// \returns the size of an array element
  boost::optional< MachineInt > is_array_access(
      ar::Statement* stmt,
      const value::AbstractDomain& inv,
      const IntInterval& offset_intv,
      const PointsToSet& addrs) const;

  /// \brief Display a memory access check, if requested
  bool display_mem_access_check(Result result,
                                ar::Statement* stmt,
                                ar::Value* pointer,
                                ar::Value* access_size) const;

  /// \brief Display a memory access check, if requested
  bool display_mem_access_check(Result result,
                                ar::Statement* stmt,
                                ar::Value* pointer,
                                ar::Value* access_size,
                                MemoryLocation* addr) const;

  /// \brief Display a strcpy() check, if requested
  bool display_strcpy_check(Result result,
                            ar::Statement* stmt,
                            ar::Value* dest_op,
                            ar::Value* src_op) const;

}; // end class BufferOverflowChecker

} // end namespace analyzer
} // end namespace ikos
