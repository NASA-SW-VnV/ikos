/*******************************************************************************
 *
 * \file
 * \brief Memory watch checker
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

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Memory watch checker
class MemoryWatchChecker final : public Checker {
private:
  using PointsToSet = core::PointsToSet< MemoryLocation* >;
  using IntPredicate = core::machine_int::Predicate;
  using IntUnaryOperator = core::machine_int::UnaryOperator;
  using IntBinaryOperator = core::machine_int::BinaryOperator;

private:
  //// \brief The AR context
  ar::Context& _ar_context;

  /// \brief The data layout
  const ar::DataLayout& _data_layout;

  /// \brief The unsigned integer type with the bit-width of a pointer
  ar::IntegerType* _size_type;

  /// \brief Watched pointer variable
  Variable* _watch_mem_ptr;

  /// \brief Watched size variable
  Variable* _watch_mem_size;

public:
  /// \brief Constructor
  explicit MemoryWatchChecker(Context& ctx);

  /// \brief Get the checker name
  CheckerName name() const override;

  /// \brief Get the checker description
  const char* description() const override;

  /// \brief Check a statement
  void check(ar::Statement* stmt,
             const value::AbstractDomain& inv,
             CallContext* call_context) override;

private:
  /// \brief Check a store
  void check_store(ar::Store* store, const value::AbstractDomain& inv);

  /// \brief Check a function call
  void check_call(ar::CallBase* call,
                  const value::AbstractDomain& inv,
                  CallContext* call_context);

  /// \brief Check a recursive function call
  void check_recursive_call(ar::CallBase* call,
                            ar::Function* fun,
                            const value::AbstractDomain& inv);

  /// \brief Check an intrinsic function call
  void check_intrinsic_call(ar::CallBase* call,
                            ar::Function* fun,
                            const value::AbstractDomain& inv);

  /// \brief Check a call to an unknown extern function
  void check_unknown_extern_call(ar::CallBase* call,
                                 const value::AbstractDomain& inv);

  /// \brief Check a call to an unknown function
  ///
  /// Check if the given function call might write on a watched memory location
  ///
  /// \param call
  ///   The call statement
  /// \param inv
  ///   The invariant
  /// \param may_write_params
  ///   True if the function call might write on a pointer parameter
  /// \param ignore_unknown_write
  ///   True to ignore writes on unknown pointer parameters (unsound)
  /// \param may_write_globals
  ///   True if the function call might update a global variable
  /// \param may_throw_exc
  ///   True if the function call might throw an exception
  void check_unknown_call(ar::CallBase* call,
                          value::AbstractDomain inv,
                          bool may_write_params,
                          bool ignore_unknown_write,
                          bool may_write_globals,
                          bool may_throw_exc);

  /// \brief Check a memory write
  ///
  /// Check if the given statement might write on a watched memory location
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param access_size The access size operand (in bytes)
  /// \param inv The invariant
  void check_mem_write(ar::Statement* stmt,
                       ar::Value* pointer,
                       ar::Value* access_size,
                       value::AbstractDomain inv);

  /// \brief Check a memory write
  ///
  /// Check if the given statement might write on a watched memory location
  ///
  /// \param stmt The statement
  /// \param pointer The pointer operand
  /// \param inv The invariant
  void check_mem_write(ar::Statement* stmt,
                       ar::Value* pointer,
                       value::AbstractDomain inv);

  /// \brief Initialize global variable pointers and function pointers
  void init_global_ptr(value::AbstractDomain& inv, ar::Value* value) const;

  /// \brief Return the store size for the given type, as an integer constant
  ar::IntegerConstant* store_size(ar::Type*) const;

}; // end class MemoryWatchChecker

} // end namespace analyzer
} // end namespace ikos
