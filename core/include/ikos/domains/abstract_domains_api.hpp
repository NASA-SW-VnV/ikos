/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for abstract domains.
 *
 * Author: Maxime Arthaud
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

#ifndef IKOS_ABSTRACT_DOMAINS_API_HPP
#define IKOS_ABSTRACT_DOMAINS_API_HPP

#include <string>

#include <ikos/common/types.hpp>

namespace ikos {

/// \brief Base class for abstract domains
class abstract_domain : public writeable {
public:
  /*
   * abstract domains should implement the following methods:
   *
   * // return top
   * static T top();
   *
   * // return bottom
   * static T bottom();
   *
   * // semantic inclusion
   * bool operator<=(T o);
   *
   * // join operator
   * T operator|(T o);
   *
   * // widening operator
   * T operator||(T o);
   *
   * // join operator on a loop head
   * //
   * // Example: pre_in join_loop pre_back
   * //
   * // In most cases, this is equivalent to the join operator
   * T join_loop(T o);
   *
   * // join operator on two consecutive iterations of a fix-point algorithm
   * //
   * // Example: x(n) join_iter x(n+1)
   * //
   * // In most cases, this is equivalent to the join operator
   * T join_iter(T o);
   *
   * // meet operator
   * T operator&(T);
   *
   * // narrowing operator
   * T operator&&(T o);
   *
   * // return the domain name
   * static std::string domain_name();
   *
   * note: T is the concrete class type.
   */

  /// \brief Check if the abstract value is bottom.
  ///
  /// \returns true if the abstract value is bottom, false otherwise.
  virtual bool is_bottom() = 0;

  /// \brief Check if the abstract value is top.
  ///
  /// \returns true if the abstract value is top, false otherwise.
  virtual bool is_top() = 0;

  virtual ~abstract_domain() {}

}; // end class abstract_domain

} // end namespace ikos

#endif // IKOS_ABSTRACT_DOMAINS_API_HPP
