/*******************************************************************************
 *
 * \file
 * \brief CallContext and CallContextFactory
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

#include <memory>

#include <boost/thread/shared_mutex.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/ar/semantic/statement.hpp>

#include <ikos/analyzer/support/assert.hpp>

namespace ikos {
namespace analyzer {

/// \brief Represents a calling context
class CallContext {
private:
  /// \brief Parent call context
  CallContext* _parent = nullptr;

  /// \brief Call statement
  ar::CallBase* _call = nullptr;

private:
  /// \brief Create an empty call context
  CallContext() = default;

  /// \brief Create a call context
  CallContext(CallContext* parent, ar::CallBase* call)
      : _parent(parent), _call(call) {
    ikos_assert(this->_parent != nullptr);
    ikos_assert(this->_call != nullptr);
  }

public:
  /// \brief No copy constructor
  CallContext(const CallContext&) = delete;

  /// \brief No move constructor
  CallContext(CallContext&&) = delete;

  /// \brief No copy assignment operator
  CallContext& operator=(const CallContext&) = delete;

  /// \brief No move assignment operator
  CallContext& operator=(CallContext&&) = delete;

  /// \brief Destructor
  ~CallContext() = default;

  /// \brief Return true if this is an empty calling context
  bool empty() const { return this->_parent == nullptr; }

  /// \brief Return true if the calling context has a parent context
  bool has_parent() const { return this->_parent != nullptr; }

  /// \brief Return the parent calling context
  CallContext* parent() const {
    ikos_assert_msg(!this->empty(), "call context is empty");
    return this->_parent;
  }

  /// \brief Return the call statement leading to this context
  ar::CallBase* call() const {
    ikos_assert_msg(!this->empty(), "call context is empty");
    return this->_call;
  }

  /// \brief Return true if the given function is within the call context
  bool contains(ar::Function* fun) const {
    const CallContext* context = this;

    for (; context->_parent != nullptr; context = context->_parent) {
      if (context->_call->code()->function_or_null() == fun) {
        return true;
      }
    }

    return false;
  }

private:
  friend class CallContextFactory;

}; // end class CallContext

/// \brief Management of calling contexts
class CallContextFactory {
private:
  boost::shared_mutex _mutex;

  llvm::DenseMap< std::pair< CallContext*, ar::CallBase* >,
                  std::unique_ptr< CallContext > >
      _map;

  std::unique_ptr< CallContext > _empty_call_context;

public:
  /// \brief Constructor
  CallContextFactory();

  /// \brief No copy constructor
  CallContextFactory(const CallContextFactory&) = delete;

  /// \brief No move constructor
  CallContextFactory(CallContextFactory&&) = delete;

  /// \brief No copy assignment operator
  CallContextFactory& operator=(const CallContextFactory&) = delete;

  /// \brief No move assignment operator
  CallContextFactory& operator=(CallContextFactory&&) = delete;

  /// \brief Destructor
  ~CallContextFactory();

  /// \brief Get the empty call context
  CallContext* get_empty() { return this->_empty_call_context.get(); }

  /// \brief Get or Create the call context with the given parameters
  ///
  /// \param parent Parent call context
  /// \param call Call statement
  CallContext* get_context(CallContext* parent, ar::CallBase* call);

}; // end class CallContextFactory

} // end namespace analyzer
} // end namespace ikos
