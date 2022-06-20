/*******************************************************************************
 *
 * \file
 * \brief Implementation of MemoryLocation
 *
 * Author: Clement Decoodt
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

#include <boost/thread/locks.hpp>

#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/util/source_location.hpp>

namespace ikos {
namespace analyzer {

// MemoryLocation

MemoryLocation::MemoryLocation(MemoryLocationKind kind) : _kind(kind) {}

MemoryLocation::~MemoryLocation() = default;

// LocalMemoryLocation

LocalMemoryLocation::LocalMemoryLocation(ar::LocalVariable* var)
    : MemoryLocation(LocalMemoryKind), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void LocalMemoryLocation::dump(std::ostream& o) const {
  o << "LocalMemoryLocation{";
  this->_var->dump(o);
  o << "}";
}

// GlobalMemoryLocation

GlobalMemoryLocation::GlobalMemoryLocation(ar::GlobalVariable* var)
    : MemoryLocation(GlobalMemoryKind), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void GlobalMemoryLocation::dump(std::ostream& o) const {
  o << "GlobalMemoryLocation{";
  this->_var->dump(o);
  o << "}";
}

// FunctionMemoryLocation

FunctionMemoryLocation::FunctionMemoryLocation(ar::Function* fun)
    : MemoryLocation(FunctionMemoryKind), _fun(fun) {
  ikos_assert(this->_fun != nullptr);
}

void FunctionMemoryLocation::dump(std::ostream& o) const {
  o << "@" << this->_fun->name();
}

// AggregateMemoryLocation

AggregateMemoryLocation::AggregateMemoryLocation(ar::InternalVariable* var)
    : MemoryLocation(AggregateMemoryKind), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void AggregateMemoryLocation::dump(std::ostream& o) const {
  o << "AggregateMemoryLocation{";
  this->_var->dump(o);
  o << "}";
}

// AbsoluteZeroMemoryLocation

AbsoluteZeroMemoryLocation::AbsoluteZeroMemoryLocation()
    : MemoryLocation(AbsoluteZeroMemoryKind) {}

void AbsoluteZeroMemoryLocation::dump(std::ostream& o) const {
  o << "absolute_zero";
}

// ArgvMemoryLocation

ArgvMemoryLocation::ArgvMemoryLocation() : MemoryLocation(ArgvMemoryKind) {}

void ArgvMemoryLocation::dump(std::ostream& o) const {
  o << "argv";
}

// LibcErrnoMemoryLocation

LibcErrnoMemoryLocation::LibcErrnoMemoryLocation()
    : MemoryLocation(LibcErrnoMemoryKind) {}

void LibcErrnoMemoryLocation::dump(std::ostream& o) const {
  o << "libc.errno";
}

// DynAllocMemoryLocation

DynAllocMemoryLocation::DynAllocMemoryLocation(ar::CallBase* call,
                                               CallContext* context)
    : MemoryLocation(DynAllocMemoryKind), _call(call), _context(context) {
  ikos_assert(this->_call != nullptr && this->_context != nullptr);
}

void DynAllocMemoryLocation::dump(std::ostream& o) const {
  ikos_assert_msg(this->_call->code()->is_function_body(),
                  "dynamic allocation in global variable initializer");
  auto fun = this->_call->code()->function();
  o << "dyn_alloc:" << fun->name() << ":";

  SourceLocation loc = source_location(this->_call);
  if (loc) {
    o << loc.line() << ":" << loc.column();
  } else {
    o << this->_call;
  }

  if (!this->_context->empty()) {
    o << ":" << this->_context;
  }
}

// MemoryFactory

MemoryFactory::MemoryFactory()
    : _absolute_zero(std::make_unique< AbsoluteZeroMemoryLocation >()),
      _argv(std::make_unique< ArgvMemoryLocation >()),
      _libc_errno(std::make_unique< LibcErrnoMemoryLocation >()) {}

MemoryFactory::~MemoryFactory() = default;

LocalMemoryLocation* MemoryFactory::get_local(ar::LocalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_local_memory_mutex);
    auto it = this->_local_memory_map.find(var);
    if (it != this->_local_memory_map.end()) {
      return it->second.get();
    }
  }

  auto ml = std::make_unique< LocalMemoryLocation >(var);

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_local_memory_mutex);
    auto res = this->_local_memory_map.try_emplace(var, std::move(ml));
    return res.first->second.get();
  }
}

GlobalMemoryLocation* MemoryFactory::get_global(ar::GlobalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_global_memory_mutex);
    auto it = this->_global_memory_map.find(var);
    if (it != this->_global_memory_map.end()) {
      return it->second.get();
    }
  }

  auto ml = std::make_unique< GlobalMemoryLocation >(var);

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_global_memory_mutex);
    auto res = this->_global_memory_map.try_emplace(var, std::move(ml));
    return res.first->second.get();
  }
}

FunctionMemoryLocation* MemoryFactory::get_function(ar::Function* fun) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_function_memory_mutex);
    auto it = this->_function_memory_map.find(fun);
    if (it != this->_function_memory_map.end()) {
      return it->second.get();
    }
  }

  auto ml = std::make_unique< FunctionMemoryLocation >(fun);

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_function_memory_mutex);
    auto res = this->_function_memory_map.try_emplace(fun, std::move(ml));
    return res.first->second.get();
  }
}

FunctionMemoryLocation* MemoryFactory::get_function(
    ar::FunctionPointerConstant* cst) {
  return this->get_function(cst->function());
}

AggregateMemoryLocation* MemoryFactory::get_aggregate(
    ar::InternalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_aggregate_memory_mutex);
    auto it = this->_aggregate_memory_map.find(var);
    if (it != this->_aggregate_memory_map.end()) {
      return it->second.get();
    }
  }

  auto ml = std::make_unique< AggregateMemoryLocation >(var);

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_aggregate_memory_mutex);
    auto res = this->_aggregate_memory_map.try_emplace(var, std::move(ml));
    return res.first->second.get();
  }
}

AbsoluteZeroMemoryLocation* MemoryFactory::get_absolute_zero() {
  return this->_absolute_zero.get();
}

ArgvMemoryLocation* MemoryFactory::get_argv() {
  return this->_argv.get();
}

LibcErrnoMemoryLocation* MemoryFactory::get_libc_errno() {
  return this->_libc_errno.get();
}

DynAllocMemoryLocation* MemoryFactory::get_dyn_alloc(ar::CallBase* call,
                                                     CallContext* context) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_dyn_alloc_mutex);
    auto it = this->_dyn_alloc_map.find({call, context});
    if (it != this->_dyn_alloc_map.end()) {
      return it->second.get();
    }
  }

  auto ml = std::make_unique< DynAllocMemoryLocation >(call, context);

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_dyn_alloc_mutex);
    auto res = this->_dyn_alloc_map.try_emplace({call, context}, std::move(ml));
    return res.first->second.get();
  }
}

} // end namespace analyzer
} // end namespace ikos
