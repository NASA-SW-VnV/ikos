/**************************************************************************/ /**
 *
 * \file
 * \brief Wrapper of a discrete domain customized for GEN/KILL problems.
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

#ifndef IKOS_DATAFLOW_DOMAIN_HPP
#define IKOS_DATAFLOW_DOMAIN_HPP

#include <ikos/domains/discrete_domains.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

namespace ikos {

/// \brief Dataflow abstract domain
template < typename Element >
class dataflow_domain : public abstract_domain {
private:
  typedef dataflow_domain< Element > dataflow_domain_t;

private:
  typedef discrete_domain< Element > discrete_domain_t;

public:
  typedef typename discrete_domain_t::iterator iterator;

private:
  discrete_domain_t _inv;

public:
  static dataflow_domain_t top() {
    return dataflow_domain(discrete_domain_t::top());
  }

  static dataflow_domain_t bottom() {
    return dataflow_domain(discrete_domain_t::bottom());
  }

public:
  dataflow_domain() : _inv(discrete_domain_t::top()) {}

  dataflow_domain(Element e) : _inv(e) {}

  dataflow_domain(discrete_domain_t inv) : _inv(inv) {}

  dataflow_domain(const dataflow_domain_t& other) : _inv(other._inv) {}

  dataflow_domain_t& operator=(dataflow_domain_t other) {
    this->_inv = other._inv;
    return *this;
  }

  iterator begin() { return this->_inv.begin(); }

  iterator end() { return this->_inv.end(); }

  unsigned size() { return this->_inv.size(); }

  bool is_bottom() { return this->_inv.is_bottom(); }

  bool is_top() { return this->_inv.is_top(); }

  bool operator<=(dataflow_domain_t other) {
    return (this->_inv <= other._inv);
  }

  // set difference
  void operator-=(dataflow_domain_t x) {
    if (!x._inv.is_top()) {
      for (iterator it = x.begin(), et = x.end(); it != et; ++it) {
        this->_inv -= *it;
      }
    }
  }

  void operator|=(dataflow_domain_t x) { this->_inv = (this->_inv | x._inv); }

  dataflow_domain_t operator|(dataflow_domain_t other) {
    return this->_inv | other._inv;
  }

  dataflow_domain_t operator||(dataflow_domain_t after) { return after; }

  dataflow_domain_t join_loop(dataflow_domain_t other) {
    return this->operator|(other);
  }

  dataflow_domain_t join_iter(dataflow_domain_t other) {
    return this->operator|(other);
  }

  dataflow_domain_t operator&(dataflow_domain_t other) {
    return this->_inv & other._inv;
  }

  dataflow_domain_t operator&&(dataflow_domain_t after) { return after; }

  void write(std::ostream& o) { this->_inv.write(o); }

  static std::string domain_name() { return "Dataflow"; }

}; // end class dataflow_domain

} // end namespace ikos

#endif // IKOS_DATAFLOW_DOMAIN_HPP
