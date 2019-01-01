/*******************************************************************************
 *
 * \file
 * \brief Dot format for the abstract representation, implementation
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

#include <ikos/ar/format/dot.hpp>
#include <ikos/ar/format/text.hpp>

namespace ikos {
namespace ar {

void DotFormatter::format(std::ostream& o, Function* f) const {
  if (f->is_declaration()) {
    return;
  }

  o << "digraph \"CFG for '" << f->name() << "' function\" {\n"
    << "\tlabel=\"CFG for '" << f->name() << "' function\";\n"
    << "\n";
  this->format(o, f->body());
  o << "}\n";
}

void DotFormatter::format(std::ostream& o, GlobalVariable* gv) const {
  if (gv->is_declaration()) {
    return;
  }

  o << "digraph \"CFG for initializer of '" << gv->name() << "'\" {\n"
    << "\tlabel=\"CFG for initializer of '" << gv->name() << "'\";\n"
    << "\n";
  this->format(o, gv->initializer());
  o << "}\n";
}

void DotFormatter::format(std::ostream& o, Code* code) const {
  Namer namer(code);
  for (auto it = code->begin(), et = code->end(); it != et; ++it) {
    this->format(o, *it, namer);
  }
}

void DotFormatter::format(std::ostream& o,
                          BasicBlock* bb,
                          const Namer& namer) const {
  // Preambule
  o << "\tNode" << bb << "  [shape=record,label=\"{"
    << "#" << namer.name(bb) << ":\\l";

  // Statements
  TextFormatter formatter(this->_opts);
  for (auto it = bb->begin(), et = bb->end(); it != et; ++it) {
    o << " ";

    // Translate a statement, and escape forbidden characters
    std::ostringstream buf;
    formatter.format(buf, *it, namer);
    o << armor(buf.str());

    o << "\\l";
  }

  // Successors
  o << "}\"];\n";
  for (auto it = bb->successor_begin(), et = bb->successor_end(); it != et;
       ++it) {
    o << "\tNode" << bb << " -> Node" << *it << ";\n";
  }
}

std::string DotFormatter::armor(StringRef s) {
  std::string buffer;
  buffer.reserve(s.size() + 1);

  for (char c : s) {
    switch (c) {
      case '{':
      case '}': {
        buffer.push_back('\\');
        buffer.push_back(c);
      } break;
      default: {
        buffer.push_back(c);
      }
    }
  }

  return buffer;
}

} // end namespace ar
} // end namespace ikos
