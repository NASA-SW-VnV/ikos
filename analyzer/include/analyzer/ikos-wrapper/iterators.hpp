/*******************************************************************************
 *
 * Forward and backward fixpoint API based on IKOS
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef ANALYZER_ITERATORS_HPP
#define ANALYZER_ITERATORS_HPP

#include <boost/noncopyable.hpp>

#include <ikos/iterators/fwd_fixpoint_iterators.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/utils/demangle.hpp>

namespace analyzer {

class fixpoint_iterator_error : public analyzer_error {
public:
  fixpoint_iterator_error(const std::string& msg) : analyzer_error(msg) {}
};

class cfg_not_reversible_error : public fixpoint_iterator_error {
public:
  cfg_not_reversible_error()
      : fixpoint_iterator_error("control flow graph not reversible") {}
};

// Wrapper for ikos forward fixpoint iterator
template < typename NodeName, typename CFG, typename AbsDomain >
class fwd_fixpoint_iterator
    : public ikos::interleaved_fwd_fixpoint_iterator< NodeName,
                                                      CFG,
                                                      AbsDomain >,
      public boost::noncopyable {
protected:
  // flag to indicate when checks can be done
  bool _is_context_stable;

public:
  fwd_fixpoint_iterator(CFG cfg, bool is_context_stable)
      : ikos::interleaved_fwd_fixpoint_iterator< NodeName, CFG, AbsDomain >(
            cfg),
        _is_context_stable(is_context_stable) {}

  // forward analysis of a cfg node starting from the pre
  // returns the post invariant at the end of the node
  virtual AbsDomain analyze(NodeName node_name, AbsDomain pre) = 0;

  // process the pre invariant at a given node when the fixpoint is reached
  // and the context is stable
  virtual void check_pre(NodeName node_name, AbsDomain pre) = 0;

  // process the post invariant at a given node when the fixpoint is reached
  // and the context is stable
  virtual void check_post(NodeName node_name, AbsDomain post) = 0;

  // process the pre invariant at a given node when the fixpoint is reached
  virtual void process_pre(NodeName node_name, AbsDomain pre) {
    if (this->_is_context_stable) {
#ifdef DEBUG
      std::cerr << "Invariant at the entry of "
                << demangle(this->get_cfg().func_name()) << "::" << node_name
                << " : " << pre << std::endl;
#endif
      check_pre(node_name, pre);
    }
  }

  // process the post invariant at a given node when the fixpoint is reached
  virtual void process_post(NodeName node_name, AbsDomain post) {
    if (this->_is_context_stable) {
#ifdef DEBUG
      std::cerr << "Invariant at the exit of "
                << demangle(this->get_cfg().func_name()) << "::" << node_name
                << " : " << post << std::endl;
#endif
      check_post(node_name, post);
    }
  }

  virtual ~fwd_fixpoint_iterator() {}

}; // end fwd_fixpoint_iterator class

// A naive backward fixpoint by computing a forward fixpoint on the
// reversed CFG.
template < typename NodeName, typename CFG, typename AbsDomain >
class backward_fixpoint_iterator
    : private ikos::interleaved_fwd_fixpoint_iterator< NodeName,
                                                       CFG,
                                                       AbsDomain >,
      public boost::noncopyable {
private:
  typedef ikos::interleaved_fwd_fixpoint_iterator< NodeName, CFG, AbsDomain >
      fwd_fixpoint_iterator_t;

protected:
  bool _is_context_stable;

private:
  bool _is_reversible;

private:
  CFG reverse(const CFG& cfg) {
    CFG rev_cfg(cfg);
    this->_is_reversible = rev_cfg.reverse();
    return rev_cfg;
  }

public:
  backward_fixpoint_iterator(CFG cfg, bool is_context_stable)
      : fwd_fixpoint_iterator_t(reverse(cfg)),
        _is_context_stable(is_context_stable) {}

  CFG get_cfg() { return fwd_fixpoint_iterator_t::get_cfg(); }

  void run(AbsDomain inv) {
    if (this->_is_reversible) {
      fwd_fixpoint_iterator_t::run(inv);
    } else {
      throw cfg_not_reversible_error();
    }
  }

  // backward analysis of a cfg node starting from the post
  virtual AbsDomain analyze(NodeName node_name, AbsDomain post) = 0;

  // process the pre invariant at a given node when the fixpoint is reached
  // and the context is stable
  virtual void check_pre(NodeName node_name, AbsDomain pre) = 0;

  // process the post invariant at a given node when the fixpoint is reached
  // and the context is stable
  virtual void check_post(NodeName node_name, AbsDomain post) = 0;

  // process the pre invariant at a given node when the fixpoint is reached
  virtual void process_pre(NodeName node_name, AbsDomain post) {
    if (this->_is_context_stable) {
#ifdef DEBUG
      std::cerr << "Invariant at the exit of "
                << demangle(this->get_cfg().func_name()) << "::" << node_name
                << " : " << post << std::endl;
#endif
      check_post(node_name, post);
    }
  }

  // process the post invariant at a given node when the fixpoint is reached
  virtual void process_post(NodeName node_name, AbsDomain pre) {
    if (this->_is_context_stable) {
#ifdef DEBUG
      std::cerr << "Invariant at the entry of "
                << demangle(this->get_cfg().func_name()) << "::" << node_name
                << " : " << pre << std::endl;
#endif
      check_pre(node_name, pre);
    }
  }

  virtual ~backward_fixpoint_iterator() {}

}; // end backward_fixpoint_iterator class

} // end namespace analyzer

#endif // ANALYZER_ITERATORS_HPP
