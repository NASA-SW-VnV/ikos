/*******************************************************************************
 *
 * \file
 * \brief Color utilities
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

namespace ikos {
namespace analyzer {

/// \brief Terminal colors
enum class TermColor {
  Grey = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Magenta = 5,
  Cyan = 6,
  White = 7,
};

/// \brief Terminal attributes
enum class TermAttribute {
  Bold = 1,
  Dark = 2,
  Underline = 4,
  Blink = 5,
  Reverse = 7,
  Concealed = 8,
};

namespace color {

/// \brief Is color enabled?
extern bool Enable;

/// \brief Return a control escape sequence, or empty string if colors are
/// disabled
inline const char* if_enabled(const char* s) {
  return Enable ? s : "";
}

inline const char* off() {
  return if_enabled("\x1B[0m");
}

inline const char* bold() {
  return if_enabled("\x1B[1m");
}

inline const char* grey() {
  return if_enabled("\x1B[30m");
}
inline const char* red() {
  return if_enabled("\x1B[31m");
}
inline const char* green() {
  return if_enabled("\x1B[32m");
}
inline const char* yellow() {
  return if_enabled("\x1B[33m");
}
inline const char* blue() {
  return if_enabled("\x1B[34m");
}
inline const char* magenta() {
  return if_enabled("\x1B[35m");
}
inline const char* cyan() {
  return if_enabled("\x1B[36m");
}
inline const char* white() {
  return if_enabled("\x1B[37m");
}

inline const char* bold_grey() {
  return if_enabled("\x1B[1;30m");
}
inline const char* bold_red() {
  return if_enabled("\x1B[1;31m");
}
inline const char* bold_green() {
  return if_enabled("\x1B[1;32m");
}
inline const char* bold_yellow() {
  return if_enabled("\x1B[1;33m");
}
inline const char* bold_blue() {
  return if_enabled("\x1B[1;34m");
}
inline const char* bold_magenta() {
  return if_enabled("\x1B[1;35m");
}
inline const char* bold_cyan() {
  return if_enabled("\x1B[1;36m");
}
inline const char* bold_white() {
  return if_enabled("\x1B[1;37m");
}

inline const char* on_grey() {
  return if_enabled("\x1B[40m");
}
inline const char* on_red() {
  return if_enabled("\x1B[41m");
}
inline const char* on_green() {
  return if_enabled("\x1B[42m");
}
inline const char* on_yellow() {
  return if_enabled("\x1B[43m");
}
inline const char* on_blue() {
  return if_enabled("\x1B[44m");
}
inline const char* on_magenta() {
  return if_enabled("\x1B[45m");
}
inline const char* on_cyan() {
  return if_enabled("\x1B[46m");
}
inline const char* on_white() {
  return if_enabled("\x1B[47m");
}

} // end namespace color

} // end namespace analyzer
} // end namespace ikos
