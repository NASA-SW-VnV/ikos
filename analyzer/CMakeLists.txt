#*******************************************************************************
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2017 United States Government as represented by the
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
#*****************************************************************************/

cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)

project(ikos-analyzer)

#
# Build settings
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message (FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE STRING "Install directory." FORCE)
endif()
message(STATUS "All libraries and binaries will be installed in ${CMAKE_INSTALL_PREFIX}")

#
# CMake policy
#

# Use @rpath on Mac OS X (requires MAC OS X >= 10.5)
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
else ()
  set(CMAKE_MACOSX_RPATH ON)
endif()

# For add_cxx_flag
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

# Do not build anything when running make install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

#
# CMake utils
#

include(CheckCXXCompilerFlag)

macro(add_cxx_flag flag name)
  CHECK_CXX_COMPILER_FLAG(${flag} "CXX-SUPPORTS-${name}")
  if("CXX-SUPPORTS-${name}")
    add_definitions(${flag})
  else()
    message(FATAL_ERROR "${flag} flag is not supported by ${CMAKE_CXX_COMPILER}")
  endif()
endmacro()

#
# Dependency checks
#

# Add path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation.")
if (CUSTOM_BOOST_ROOT)
  set(BOOST_ROOT "${CUSTOM_BOOST_ROOT}")
  set(Boost_NO_SYSTEM_PATHS "ON")
endif()

find_package(Boost 1.55.0 REQUIRED
             COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})
include_directories(${GMPXX_INCLUDE_DIR})

find_package(SQLITE3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIR})

find_package(PythonInterp REQUIRED)

find_package(RT)
if (NOT RT_FOUND)
  set(RT_LIB "")
endif()

find_package(LLVM REQUIRED)

if ((LLVM_VERSION VERSION_LESS "4.0") OR (NOT (LLVM_VERSION VERSION_LESS "5.0")))
  message(FATAL_ERROR "LLVM 4 is required.")
endif()

find_package(IKOS REQUIRED)
include_directories(${IKOS_INCLUDE_DIR})

find_package(ARBOS REQUIRED)
include_directories(${ARBOS_INCLUDE_DIR})

#
# Compiler flags
#

add_cxx_flag("-std=c++1y" "CXX14")
add_cxx_flag("-Wall" "WALL")
# warning flags from -Wextra
add_cxx_flag("-Wempty-body" "WEMPTY_BODY")
add_cxx_flag("-Wmissing-field-initializers" "WMISSING_FIELD_INITIALIZERS")
add_cxx_flag("-Wsign-compare" "WSIGN_COMPARE")
add_cxx_flag("-Wtype-limits" "WTYPE_LIMITS")
add_cxx_flag("-Wuninitialized" "WUNINITIALIZED")
add_definitions("-DBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS")

#
# Analysis options
#

# Choose the numerical abstract domain for the analysis
# Available choices: INTERVAL, CONGRUENCE, INTERVAL_CONGRUENCE, OCTAGON, DBM,
# VAR_PACKING_DBM, VAR_PACKING_DBM_CONGRUENCE, GAUGE, GAUGE_INTERVAL_CONGRUENCE,
# APRON_INTERVAL, APRON_OCTAGON, APRON_POLKA_POLYHEDRA,
# APRON_POLKA_LINEAR_EQUALITIES, APRON_PPL_POLYHEDRA,
# APRON_PPL_LINEAR_CONGRUENCES, APRON_PKGRID_POLYHEDRA_LIN_CONGRUENCES
set(ABSTRACT_DOMAIN "INTERVAL" CACHE STRING "Numerical abstract domain used by the analysis.")
add_definitions("-D${ABSTRACT_DOMAIN}_DOMAIN")

if (ABSTRACT_DOMAIN MATCHES "^APRON_")
  find_package(APRON REQUIRED)
  include_directories(${APRON_INCLUDE_DIR})
endif()

#
# Targets
#

include_directories(include)

# arbos optimization passes
add_library(ar-to-dot SHARED src/ar-passes/ar_to_dot.cpp)
target_link_libraries(ar-to-dot ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS ar-to-dot DESTINATION lib OPTIONAL)

add_library(inline-init-gv SHARED src/ar-passes/inline_init_gv.cpp)
target_link_libraries(inline-init-gv ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS inline-init-gv DESTINATION lib OPTIONAL)

add_library(pointer-shift-opt SHARED src/ar-passes/pointer_shift_opt.cpp)
target_link_libraries(pointer-shift-opt ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS pointer-shift-opt DESTINATION lib OPTIONAL)

add_library(add-loop-counters SHARED src/ar-passes/add_loop_counters.cpp)
target_link_libraries(add-loop-counters ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS add-loop-counters DESTINATION lib OPTIONAL)

add_library(branching-opt SHARED src/ar-passes/branching_opt.cpp)
target_link_libraries(branching-opt ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS branching-opt DESTINATION lib OPTIONAL)

add_library(unify-exit-nodes SHARED src/ar-passes/unify_exit_nodes.cpp)
target_link_libraries(unify-exit-nodes ${ARBOS_LIB} ${Boost_LIBRARIES})
install(TARGETS unify-exit-nodes DESTINATION lib OPTIONAL)

# analysis passes
add_library(analyzer SHARED src/ar-passes/analyzer.cpp)
target_link_libraries(analyzer ${ARBOS_LIB} ${GMPXX_LIB} ${GMP_LIB} ${SQLITE3_LIB} ${Boost_LIBRARIES})
if (APRON_FOUND)
  target_link_libraries(analyzer ${APRON_LIBRARIES})
endif()
install(TARGETS analyzer DESTINATION lib OPTIONAL)

add_library(cpa SHARED EXCLUDE_FROM_ALL src/ar-passes/cpa.cpp)
target_link_libraries(cpa ${ARBOS_LIB} ${GMPXX_LIB} ${GMP_LIB} ${SQLITE3_LIB} ${Boost_LIBRARIES})
#install(TARGETS cpa DESTINATION lib OPTIONAL)

add_library(reaching SHARED EXCLUDE_FROM_ALL src/ar-passes/reaching.cpp)
target_link_libraries(reaching ${ARBOS_LIB} ${GMPXX_LIB} ${GMP_LIB} ${SQLITE3_LIB} ${Boost_LIBRARIES})
#install(TARGETS reaching DESTINATION lib OPTIONAL)

# settings.py
configure_file(python/settings.cmake.in python/settings.cmake @ONLY)

add_custom_command(
  OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/python/ikos/settings/__init__.py"
    "[force-rebuild]" # missing file to force rebuild
  COMMAND
    ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/ikos/settings.py.in"
  VERBATIM)

# python module
configure_file(python/setup.py.in python/setup.py @ONLY)

file(GLOB PYTHON_FILES python/ikos/*.py)

add_custom_command(
  OUTPUT
    python/build/lib/ikos/__init__.py
  COMMAND
    ${PYTHON_EXECUTABLE} setup.py -q build
  WORKING_DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}/python"
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/python/setup.py"
    "${CMAKE_CURRENT_BINARY_DIR}/python/ikos/settings/__init__.py"
    "[force-rebuild]" # missing file to force rebuild
    ${PYTHON_FILES}
)
add_custom_target(python_module ALL
  DEPENDS python/build/lib/ikos/__init__.py
)

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py install
                                \"--prefix=${CMAKE_INSTALL_PREFIX}\"
                              WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/python\")")

# python scripts
configure_file(scripts/ikos.py.in scripts/ikos @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/scripts/ikos" DESTINATION bin OPTIONAL)

configure_file(scripts/ikos-render.py.in scripts/ikos-render @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/scripts/ikos-render" DESTINATION bin OPTIONAL)

configure_file(scripts/ikos-config.py.in scripts/ikos-config @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/scripts/ikos-config" DESTINATION bin OPTIONAL)

#
# Regression tests
#

include(CTest)
enable_testing()

add_test(boa "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/boa/regrtest.py")
add_test(dbz "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/dbz/regrtest.py")
add_test(mem "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/mem/regrtest.py")
add_test(null "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/null/regrtest.py")
add_test(prover "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/prover/regrtest.py")
add_test(uva "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/uva/regrtest.py")
if (ABSTRACT_DOMAIN MATCHES "CONGRUENCE")
  add_test(upa "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/upa/regrtest.py")
endif()

#
# Doxygen
#

find_package (Doxygen)
if (DOXYGEN_FOUND)
  configure_file(docs/doxygen/Doxyfile.in docs/Doxyfile @ONLY)
  add_custom_target(doxygen-analyzer
    ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating Analyzer API documentation with Doxygen" VERBATIM
  )
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs/html" DESTINATION docs/analyzer OPTIONAL)
endif ()

#
# If it's the top level CMakeLists.txt, we add some aliases
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_custom_target(docs DEPENDS doxygen-analyzer)
endif()
