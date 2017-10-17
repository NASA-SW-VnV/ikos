/*******************************************************************************
 *
 * Representation, manipulation and parsing of floating-point numbers as defined
 * in the IEEE 754 standard.
 *
 * Author: Arnaud J. Venet
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

#ifndef ARBOS_FP_HPP
#define ARBOS_FP_HPP

#include <functional>
#include <iostream>
#include <string>

namespace arbos {

class fp_error : public arbos_error {
public:
  fp_error(std::string msg) : arbos_error("[floating point] " + msg) {}

}; // class fp_error

class fp_number {
private:
  std::string _fp;

public:
  fp_number() : _fp("0") {}

  fp_number(std::string fp) : _fp(fp) {}

  void write(std::ostream& o) { o << this->_fp; }

  std::size_t hash_value() { return std::hash< std::string >()(this->_fp); }

  bool operator==(fp_number other) const { return this->_fp == other._fp; }
}; // class floating_point

inline std::ostream& operator<<(std::ostream& o, fp_number& fp) {
  fp.write(o);
  return o;
}

inline std::size_t hash_value(const fp_number& fp) {
  fp_number& f = const_cast< fp_number& >(fp);
  return f.hash_value();
}

} // end namespace arbos

namespace std {

template <>
struct hash< arbos::fp_number > {
  std::size_t operator()(const arbos::fp_number& fp) const {
    arbos::fp_number& f = const_cast< arbos::fp_number& >(fp);
    return f.hash_value();
  }
};

} // end namespace std

#endif // ARBOS_FP_HPP
