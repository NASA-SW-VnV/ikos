/**************************************************************************/ /**
 *
 * \file
 * \brief Abstract domain for reaching definitions based on dataflow domain
 *
 * Author: Jorge A. Navas
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

#ifndef IKOS_REACHING_HPP
#define IKOS_REACHING_HPP

#include <ikos/domains/dataflow_domain.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

namespace ikos {

/// \brief Reaching abstract domains
template < typename VariableName >
class reaching_domain : public abstract_domain {
private:
  typedef dataflow_domain< VariableName > dataflow_domain_t;

public:
  typedef typename dataflow_domain_t::iterator iterator;

private:
  typedef reaching_domain< VariableName > reaching_domain_t;

  dataflow_domain_t _inv;

public:
  reaching_domain(dataflow_domain_t inv) : _inv(inv) {}

  static reaching_domain_t top() {
    return reaching_domain(dataflow_domain_t::top());
  }

  static reaching_domain_t bottom() {
    return reaching_domain(dataflow_domain_t::bottom());
  }

  reaching_domain() : _inv(dataflow_domain_t::bottom()) {}

  reaching_domain(VariableName v) : _inv(v) {}

  reaching_domain(const reaching_domain_t& other) : _inv(other._inv) {}

  reaching_domain_t& operator=(reaching_domain_t other) {
    this->_inv = other._inv;
    return *this;
  }

  iterator begin() { return this->_inv.begin(); }

  iterator end() { return this->_inv.end(); }

  bool is_bottom() { return this->_inv.is_bottom(); }

  bool is_top() { return this->_inv.is_top(); }

  bool operator<=(reaching_domain_t other) {
    return (this->_inv <= other._inv);
  }

  void operator-=(reaching_domain_t x) { this->_inv -= x._inv; }

  void operator+=(reaching_domain_t x) { this->_inv |= x._inv; }

  reaching_domain_t operator|(reaching_domain_t other) {
    return reaching_domain_t(this->_inv | other._inv);
  }

  reaching_domain_t operator||(reaching_domain_t other) {
    return operator|(other);
  }

  reaching_domain_t join_loop(reaching_domain_t other) {
    return reaching_domain_t(this->_inv.join_loop(other._inv));
  }

  reaching_domain_t join_iter(reaching_domain_t other) {
    return reaching_domain_t(this->_inv.join_loop(other._inv));
  }

  reaching_domain_t operator&(reaching_domain_t other) {
    return reaching_domain_t(this->_inv & other._inv);
  }

  reaching_domain_t operator&&(reaching_domain_t other) {
    return operator&(other);
  }

  void write(std::ostream& o) { this->_inv.write(o); }

  static std::string domain_name() { return "Reaching"; }

}; // end class reaching_domain

} // end namespace ikos

#endif // IKOS_REACHING_HPP
