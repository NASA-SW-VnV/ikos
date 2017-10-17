/**************************************************************************/ /**
 *
 * \file
 * \brief Abstract domain for copy propagation based on dataflow domain.
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

#ifndef IKOS_CPA_HPP
#define IKOS_CPA_HPP

#include <ikos/algorithms/expression.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/dataflow_domain.hpp>

namespace ikos {

/// \brief Represents pairs of variable and expression
template < typename VariableName, typename Number >
class Substitution {
public:
  typedef expression< VariableName, Number > expression_t;

private:
  typedef Substitution< VariableName, Number > substitution_t;

  VariableName _v;
  expression_t _exp;

public:
  Substitution(VariableName v, expression_t exp) : _v(v), _exp(exp) {}

  Substitution(const substitution_t& other) : _v(other._v), _exp(other._exp) {}

  substitution_t& operator=(substitution_t other) {
    this->_v = other._v;
    this->_exp = other._exp;
    return *this;
  }

  VariableName variable() const { return this->_v; }

  expression_t get_expression() const { return this->_exp; }

  index64_t index() const {
    // note that we ignore the expression
    return this->_v.index();
  }

  void write(std::ostream& o) {
    o << "(" << this->_v << "," << this->_exp << ")";
  }

}; // end class Substitution

/// \brief Copy Propagation abstract domain
///
/// This domain is a must domain so the confluence operator is the
/// meet, top is the most precise representation, and bottom is the
/// least precise. a <= b means that a is **less precise** than b.  The
/// entry block is initialized optimistically (i.e., all the copy
/// assignments are available) with top.
///
/// If top is the explicit set of all possible copy assignments in the
/// program nothing else is needed. However, we represent top
/// implicitly so this makes a bit complicated things, in particular
/// operator+= and operator-=.
template < typename VariableName, typename Number >
class cpa_domain : public abstract_domain {
public:
  typedef expression< VariableName, Number > expression_t;
  typedef Substitution< VariableName, Number > substitution_t;

private:
  typedef dataflow_domain< substitution_t > dataflow_domain_t;

public:
  typedef typename dataflow_domain_t::iterator iterator;

private:
  typedef cpa_domain< VariableName, Number > cpa_domain_t;
  dataflow_domain_t _inv;

  cpa_domain(dataflow_domain_t inv) : _inv(inv) {}

public:
  static cpa_domain_t top() { return cpa_domain(dataflow_domain_t::bottom()); }

  static cpa_domain_t bottom() { return cpa_domain(dataflow_domain_t::top()); }

  cpa_domain() : _inv(dataflow_domain_t::top()) {}

  cpa_domain(substitution_t s) : _inv(s) {}

  cpa_domain(const cpa_domain_t& other) : _inv(other._inv) {}

  cpa_domain_t& operator=(cpa_domain_t other) {
    this->_inv = other._inv;
    return *this;
  }

  iterator begin() { return this->_inv.begin(); }

  iterator end() { return this->_inv.end(); }

  bool is_bottom() { return this->_inv.is_top(); }

  bool is_top() { return this->_inv.is_bottom(); }

  bool operator<=(cpa_domain_t other) { return other._inv <= this->_inv; }

  void operator-=(cpa_domain_t x) { this->_inv -= x._inv; }

  void operator+=(cpa_domain_t x) {
    if (this->_inv.is_top())
      this->_inv = (this->_inv & x._inv);
    else
      this->_inv = (this->_inv | x._inv);
  }

  cpa_domain_t operator|(cpa_domain_t other) {
    return cpa_domain_t(this->_inv & other._inv);
  }

  cpa_domain_t operator||(cpa_domain_t after) { return after; }

  cpa_domain_t join_loop(cpa_domain_t other) { return this->operator|(other); }

  cpa_domain_t join_iter(cpa_domain_t other) { return this->operator|(other); }

  cpa_domain_t operator&(cpa_domain_t other) {
    return cpa_domain_t(this->_inv | other._inv);
  }

  cpa_domain_t operator&&(cpa_domain_t after) { return after; }

  void transfer_function(cpa_domain_t kill, cpa_domain_t gen) {
    this->operator-=(kill);
    this->operator+=(gen);
  }

  /// \returns the set of substitutions (var,*) or (*,var)
  cpa_domain_t operator[](const VariableName& var) {
    cpa_domain_t subs = cpa_domain_t::bottom();
    if (!this->_inv.is_top()) {
      for (iterator it = this->_inv.begin(); it != this->_inv.end(); ++it) {
        if ((*it).variable() == var || (*it).get_expression().variables()[var])
          subs += *it;
      }
    }
    return subs;
  }

  void write(std::ostream& o) { this->_inv.write(o); }

  static std::string domain_name() { return "Copy Propagation Analysis"; }

}; // end class cpa_domain

} // end namespace ikos

#endif // IKOS_CPA_HPP
