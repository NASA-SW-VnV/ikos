/*******************************************************************************
 *
 * \file
 * \brief Implementation of data structures for the manipulation of JSON objects
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2016-2019 United States Government as represented by the
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

#include <ikos/analyzer/json/json.hpp>

namespace ikos {
namespace analyzer {

// JsonNode

JsonNode::~JsonNode() = default;

// JsonInteger

std::string JsonInteger::str() const {
  return this->_n.str();
}

// JsonFloat

std::string JsonFloat::str() const {
  return std::to_string(this->_d);
}

// JsonBool

std::string JsonBool::str() const {
  if (this->_b) {
    return "true";
  } else {
    return "false";
  }
}

// JsonString

/// \brief Return the hexadecimal character for the given number
static char hexdigit(unsigned n, bool lower_case = false) {
  if (n < 10U) {
    return static_cast< char >('0' + n);
  } else if (n < 16U) {
    return static_cast< char >((lower_case ? 'a' : 'A') + (n - 10));
  } else {
    ikos_unreachable("invalid argument");
  }
}

std::string JsonString::str() const {
  std::string r;
  r.reserve(this->_s.size() + 2);
  r.push_back('"');
  for (const char c : this->_s) {
    if (c == '"' || c == '\\') {
      r.push_back('\\');
      r.push_back(c);
    } else if (std::isprint(c) != 0) {
      r.push_back(c);
    } else if (c == '\b') {
      r.append("\\b");
    } else if (c == '\t') {
      r.append("\\t");
    } else if (c == '\n') {
      r.append("\\n");
    } else if (c == '\f') {
      r.append("\\f");
    } else if (c == '\r') {
      r.append("\\r");
    } else {
      r.append("\\u00");
      r.push_back(hexdigit(static_cast< unsigned char >(c) >> 4U));
      r.push_back(hexdigit(static_cast< unsigned char >(c) & 0x0FU));
    }
  }
  r.push_back('"');
  return r;
}

// JsonList

std::string JsonList::str() const {
  std::string r;
  r.reserve(this->_buf.size() + 2);
  r.push_back('[');
  r.append(this->_buf);
  r.push_back(']');
  return r;
}

// JsonDict

std::string JsonDict::str() const {
  std::string r;
  r.reserve(this->_buf.size() + 2);
  r.push_back('{');
  r.append(this->_buf);
  r.push_back('}');
  return r;
}

} // end namespace analyzer
} // end namespace ikos
