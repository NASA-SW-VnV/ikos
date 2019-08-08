###############################################################################
#
# Find GMP headers and libraries.
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2019 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
# Disclaimers:
#
# No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
# ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
# TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
# ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
# ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
# THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
# ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
# RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
# RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
# DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
# IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
#
# Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
# THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
# AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
# IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
# USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
# RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
# HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
# AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
# RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
# UNILATERAL TERMINATION OF THIS AGREEMENT.
#
###############################################################################

if (NOT GMP_FOUND)
  set(GMP_ROOT "" CACHE PATH "Path to gmp install directory")

  find_path(GMP_INCLUDE_DIR
    NAMES gmp.h
    HINTS "${GMP_ROOT}/include"
    DOC "Path to gmp include directory"
  )

  find_library(GMP_LIB
    NAMES gmp
    HINTS "${GMP_ROOT}/lib"
    DOC "Path to gmp library"
  )

  find_path(GMPXX_INCLUDE_DIR
    NAMES gmpxx.h
    HINTS "${GMP_ROOT}/include"
    DOC "Path to gmpxx include directory"
  )

  find_library(GMPXX_LIB
    NAMES gmpxx
    HINTS "${GMP_ROOT}/lib"
    DOC "Path to gmpxx library"
  )

  if (GMP_INCLUDE_DIR AND GMP_LIB)
    file(WRITE "${PROJECT_BINARY_DIR}/FindGMPVersion.c" "
      #include <assert.h>
      #include <stdio.h>
      #include <gmp.h>

      int main() {
        mpz_t i, j, k;
        mpz_init_set_str(i, \"1a\", 16);
        mpz_init(j);
        mpz_init(k);
        mpz_sqrtrem(j, k, i);
        assert(mpz_get_si(j) == 5 && mpz_get_si(k) == 1);
        printf(\"%s\", gmp_version);
        return 0;
      }
    ")

    try_run(
      RUN_RESULT
      COMPILE_RESULT
      "${PROJECT_BINARY_DIR}"
      "${PROJECT_BINARY_DIR}/FindGMPVersion.c"
      CMAKE_FLAGS
        "-DINCLUDE_DIRECTORIES:STRING=${GMP_INCLUDE_DIR}"
        "-DLINK_LIBRARIES:STRING=${GMP_LIB}"
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
      RUN_OUTPUT_VARIABLE GMP_VERSION
    )

    if (NOT COMPILE_RESULT)
      message(FATAL_ERROR "error when trying to compile a program with GMP:\n${COMPILE_OUTPUT}")
    endif()
    if (RUN_RESULT)
      message(FATAL_ERROR "error when running a program linked with GMP:\n${GMP_VERSION}")
    endif()
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GMP
    REQUIRED_VARS
      GMP_INCLUDE_DIR
      GMP_LIB
      GMPXX_INCLUDE_DIR
      GMPXX_LIB
    VERSION_VAR
      GMP_VERSION
    FAIL_MESSAGE
      "Could NOT find GMP. Please provide -DGMP_ROOT=/path/to/gmp")
endif()
