/*******************************************************************************
 *
 * ARBOS pass to try options
 *
 * Authors: Maxime Arthaud
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

#include <iostream>
#include <vector>

#include <arbos/semantics/ar.hpp>

using namespace arbos;

namespace {

static Option< bool > flag("flag,f", "A boolean option");
static Option< int > integer("integer,i", "An integer option");
static Option< int > integer_def("integer-def",
                                 "An integer option with a default value",
                                 42);
static Option< std::vector< std::string > > list("list,L", "A list of strings");

class OptionsPass : public Pass {
public:
  OptionsPass() : Pass("options", "Demo pass for ARBOS options") {}
  virtual ~OptionsPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    if (flag) {
      std::cout << "flag is enabled" << std::endl;
    } else {
      std::cout << "flag disabled" << std::endl;
    }

    if (integer) {
      std::cout << "integer is " << integer << std::endl;
    } else {
      std::cout << "integer is not defined" << std::endl;
    }

    std::cout << "integer-def is " << integer_def << std::endl;

    if (list) {
      std::cout << "list is:" << std::endl;
      const std::vector< std::string >& v = list;
      for (std::vector< std::string >::const_iterator it = v.begin();
           it != v.end();
           ++it) {
        std::cout << "  " << *it << std::endl;
      }
    } else {
      std::cout << "list is not defined" << std::endl;
    }
  }
};
}

extern "C" Pass* init() {
  return new OptionsPass();
}
