#*******************************************************************************
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2023 United States Government as represented by the
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

cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

project(ikos-analyzer)

#
# Build settings
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install directory" FORCE)
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  message(STATUS "CMake version: ${CMAKE_VERSION}")
  message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
endif()

#
# Dependency checks
#

# Add path for custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(Threads REQUIRED)

set(CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation")
if (CUSTOM_BOOST_ROOT)
  set(BOOST_ROOT "${CUSTOM_BOOST_ROOT}")
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

find_package(Boost 1.55.0 REQUIRED
             COMPONENTS filesystem system thread)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(SYSTEM ${GMP_INCLUDE_DIR})
include_directories(SYSTEM ${GMPXX_INCLUDE_DIR})

find_package(TBB 2 REQUIRED)
include_directories(SYSTEM ${TBB_INCLUDE_DIRS})

find_package(SQLite3 REQUIRED)
include_directories(SYSTEM ${SQLITE3_INCLUDE_DIR})

find_package(APRON)
if (APRON_FOUND)
  include_directories(SYSTEM ${APRON_INCLUDE_DIRS})
  add_definitions("-DHAS_APRON")
endif()

find_package(PythonInterp REQUIRED)

find_package(Core REQUIRED)
include_directories(${CORE_INCLUDE_DIR})

find_package(AR REQUIRED)
include_directories(${AR_INCLUDE_DIR})

find_package(FrontendLLVM REQUIRED)
include_directories(${FRONTEND_LLVM_INCLUDE_DIR})

find_package(LLVM REQUIRED)
include_directories(SYSTEM ${LLVM_INCLUDE_DIR})

if ((LLVM_VERSION VERSION_LESS "14") OR (NOT (LLVM_VERSION VERSION_LESS "15")))
  message(FATAL_ERROR "LLVM 14 is required.")
endif()

# Add path to llvm cmake modules
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(LLVMConfig)

if ((NOT LLVM_ENABLE_RTTI) AND (NOT WIN32))
  message(WARNING "LLVM was built without run-time type information (RTTI)")
endif()

set(LLVM_ENABLE_WARNINGS TRUE)
set(LLVM_REQUIRES_EH TRUE)
set(LLVM_REQUIRES_RTTI TRUE)

include(HandleLLVMOptions)
include(AddLLVM)

option(IKOS_LINK_LLVM_DYLIB "Link IKOS against the libLLVM dynamic library" OFF)

find_package(Clang REQUIRED)

if (NOT (LLVM_VERSION VERSION_EQUAL CLANG_VERSION))
  message(FATAL_ERROR "LLVM and Clang versions do not match.")
endif()

#
# Compiler flags
#

include(AddFlagUtils)
add_compiler_flag(REQUIRED "CXX14" "-std=c++1y")
add_compiler_flag(REQUIRED "FVISIBILITY_INLINES_HIDDEN" "-fvisibility-inlines-hidden")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compiler_flag(REQUIRED "WEVERYTHING" "-Weverything")
  add_compiler_flag(OPTIONAL "WNO_SWITCH_ENUM" "-Wno-switch-enum")
  add_compiler_flag(OPTIONAL "WNO_PADDED" "-Wno-padded")
  add_compiler_flag(OPTIONAL "WNO_CXX98_COMPAT" "-Wno-c++98-compat")
  add_compiler_flag(OPTIONAL "WNO_CXX98_COMPAT_PEDANTIC" "-Wno-c++98-compat-pedantic")
  add_compiler_flag(OPTIONAL "WNO_C99_EXTENSIONS" "-Wno-c99-extensions")
  add_compiler_flag(OPTIONAL "WNO_COVERED_SWITCH_DEFAULT" "-Wno-covered-switch-default")
  add_compiler_flag(OPTIONAL "WNO_EXIT_TIME_DESTRUCTORS" "-Wno-exit-time-destructors")
  add_compiler_flag(OPTIONAL "WNO_GLOBAL_CONSTRUCTORS" "-Wno-global-constructors")
  add_compiler_flag(OPTIONAL "WNO_WEAK_VTABLES" "-Wno-weak-vtables")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compiler_flag(REQUIRED "WALL" "-Wall")
  add_compiler_flag(REQUIRED "WEXTRA" "-Wextra")
  add_compiler_flag(OPTIONAL "WNO_MAYBE_UNINITIALIZED" "-Wno-maybe-uninitialized")
  add_compiler_flag(OPTIONAL "WNO_REDUNDANT_MOVE" "-Wno-redundant-move")
  add_compiler_flag(OPTIONAL "WNO_UNUSED_LOCAL_TYPEDEFS" "-Wno-unused-local-typedefs")
endif()

#
# Targets
#

include_directories(include)

# install intrinsic.h
install(FILES include/ikos/analyzer/intrinsic.h
  DESTINATION include/ikos/analyzer)

# ikos-analyzer binary
add_executable(ikos-analyzer
  src/ikos_analyzer.cpp
  src/analysis/call_context.cpp
  src/analysis/fixpoint_parameters.cpp
  src/analysis/hardware_addresses.cpp
  src/analysis/literal.cpp
  src/analysis/liveness.cpp
  src/analysis/memory_location.cpp
  src/analysis/option.cpp
  src/analysis/pointer/constraint.cpp
  src/analysis/pointer/function.cpp
  src/analysis/pointer/pointer.cpp
  src/analysis/pointer/value.cpp
  src/analysis/value/abstract_domain.cpp
  src/analysis/value/global_variable.cpp
  src/analysis/value/interprocedural/concurrent/analysis.cpp
  src/analysis/value/interprocedural/concurrent/function_fixpoint.cpp
  src/analysis/value/interprocedural/init_invariant.cpp
  src/analysis/value/interprocedural/sequential/analysis.cpp
  src/analysis/value/interprocedural/sequential/function_fixpoint.cpp
  src/analysis/value/interprocedural/sequential/global_init_fixpoint.cpp
  src/analysis/value/interprocedural/sequential/progress.cpp
  src/analysis/value/intraprocedural/concurrent/analysis.cpp
  src/analysis/value/intraprocedural/concurrent/function_fixpoint.cpp
  src/analysis/value/intraprocedural/sequential/analysis.cpp
  src/analysis/value/intraprocedural/sequential/function_fixpoint.cpp
  src/analysis/value/machine_int_domain.cpp
  src/analysis/value/machine_int_domain/apron_interval.cpp
  src/analysis/value/machine_int_domain/apron_octagon.cpp
  src/analysis/value/machine_int_domain/apron_pkgrid_polyhedra_lin_cong.cpp
  src/analysis/value/machine_int_domain/apron_polka_linear_equalities.cpp
  src/analysis/value/machine_int_domain/apron_polka_polyhedra.cpp
  src/analysis/value/machine_int_domain/apron_ppl_linear_congruences.cpp
  src/analysis/value/machine_int_domain/apron_ppl_polyhedra.cpp
  src/analysis/value/machine_int_domain/congruence.cpp
  src/analysis/value/machine_int_domain/dbm.cpp
  src/analysis/value/machine_int_domain/gauge.cpp
  src/analysis/value/machine_int_domain/gauge_interval_congruence.cpp
  src/analysis/value/machine_int_domain/interval.cpp
  src/analysis/value/machine_int_domain/interval_congruence.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_octagon.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_pkgrid_polyhedra_lin_cong.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_polka_linear_equalities.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_polka_polyhedra.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_ppl_linear_congruences.cpp
  src/analysis/value/machine_int_domain/var_pack_apron_ppl_polyhedra.cpp
  src/analysis/value/machine_int_domain/var_pack_dbm.cpp
  src/analysis/value/machine_int_domain/var_pack_dbm_congruence.cpp
  src/analysis/variable.cpp
  src/analysis/widening_hint.cpp
  src/checker/assert_prover.cpp
  src/checker/buffer_overflow.cpp
  src/checker/checker.cpp
  src/checker/dead_code.cpp
  src/checker/debug.cpp
  src/checker/division_by_zero.cpp
  src/checker/double_free.cpp
  src/checker/function_call.cpp
  src/checker/int_overflow_base.cpp
  src/checker/memory_watch.cpp
  src/checker/null_dereference.cpp
  src/checker/pointer_alignment.cpp
  src/checker/pointer_compare.cpp
  src/checker/pointer_overflow.cpp
  src/checker/shift_count.cpp
  src/checker/signed_int_overflow.cpp
  src/checker/soundness.cpp
  src/checker/uninitialized_variable.cpp
  src/checker/unsigned_int_overflow.cpp
  src/database/output.cpp
  src/database/sqlite.cpp
  src/database/table.cpp
  src/database/table/call_contexts.cpp
  src/database/table/checks.cpp
  src/database/table/files.cpp
  src/database/table/functions.cpp
  src/database/table/memory_locations.cpp
  src/database/table/operands.cpp
  src/database/table/settings.cpp
  src/database/table/statements.cpp
  src/database/table/times.cpp
  src/exception.cpp
  src/json/json.cpp
  src/util/color.cpp
  src/util/log.cpp
  src/util/progress.cpp
  src/util/source_location.cpp
  src/util/timer.cpp
)
if (IKOS_LINK_LLVM_DYLIB)
  set(IKOS_ANALYZER_LLVM_LIBS "LLVM")
else()
  llvm_map_components_to_libnames(IKOS_ANALYZER_LLVM_LIBS
    core
    ipo
    irreader
    support
    transformutils
  )
endif()
target_link_libraries(ikos-analyzer
  Threads::Threads
  ${FRONTEND_LLVM_TO_AR_LIB}
  ${IKOS_ANALYZER_LLVM_LIBS}
  ${SQLITE3_LIB}
  ${Boost_LIBRARIES}
  ${GMP_LIB}
  ${GMPXX_LIB}
  ${TBB_LIBRARIES}
  ${AR_LIB}
)
if (APRON_FOUND)
  target_link_libraries(ikos-analyzer ${APRON_LIBRARIES})
endif()
install(TARGETS ikos-analyzer RUNTIME DESTINATION bin)

# python wrapper
option(APPEND_GIT_VERSION "Append the current git commit to the version number" OFF)
option(FORCE_UPDATE_VERSION "Force the update of the version on every build" OFF)

# settings/__init__.py
configure_file(python/settings.cmake.in python/settings.cmake @ONLY)

if (NOT FORCE_UPDATE_VERSION)
  add_custom_command(
    OUTPUT
      "python/ikos/settings/__init__.py"
    COMMAND
      ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/python/ikos/settings.py.in"
    VERBATIM)
else()
  add_custom_command(
    OUTPUT
      "python/ikos/settings/__init__.py"
      "[update-version]" # missing file to force rebuild
    COMMAND
      ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/python/settings.cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/python/ikos/settings.py.in"
    VERBATIM)
endif()
add_custom_target(ikos-python-settings ALL
  DEPENDS "python/ikos/settings/__init__.py"
)

# setup.py
configure_file(python/setup.py.in python/setup.py @ONLY)

option(INSTALL_PYTHON_VIRTUALENV "Install a python virtual environment for ikos" ON)
option(PYTHON_VENV_EXECUTABLE "Path to the python executable of an existing virtual environment")
if (INSTALL_PYTHON_VIRTUALENV)
  install(CODE "
    message(STATUS \"Running python -m venv ${CMAKE_INSTALL_PREFIX}/libexec\")
    execute_process(COMMAND \"${PYTHON_EXECUTABLE}\" -m venv \"${CMAKE_INSTALL_PREFIX}/libexec\")

    message(STATUS \"Running ${CMAKE_INSTALL_PREFIX}/libexec/bin/python -m pip install -U pip\")
    execute_process(COMMAND \"${CMAKE_INSTALL_PREFIX}/libexec/bin/python\" -m pip install -U pip)

    message(STATUS \"Running ${CMAKE_INSTALL_PREFIX}/libexec/bin/python -m pip install -U pygments setuptools\")
    execute_process(COMMAND \"${CMAKE_INSTALL_PREFIX}/libexec/bin/python\" -m pip install pygments setuptools)

    message(STATUS \"Running ${CMAKE_INSTALL_PREFIX}/libexec/bin/python -m pip install .\")
    execute_process(COMMAND \"${CMAKE_INSTALL_PREFIX}/libexec/bin/python\" -m pip install .
                    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/python\")
  ")
  set(PYTHON_VENV_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/libexec/bin/python")
else()
  # This can be used by the Homebrew formula or by package maintainers.
  if (NOT PYTHON_VENV_EXECUTABLE)
    message(FATAL_ERROR "Please specify -DPYTHON_VENV_EXECUTABLE=<path> when using -DINSTALL_PYTHON_VIRTUALENV=OFF")
  endif()
endif()

# python web resources
install(DIRECTORY python/ikos/view DESTINATION share/ikos)

# python scripts
configure_file(script/ikos.py.in script/ikos @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos" DESTINATION bin)

configure_file(script/ikos-config.py.in script/ikos-config @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-config" DESTINATION bin)

configure_file(script/ikos-report.py.in script/ikos-report @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-report" DESTINATION bin)

configure_file(script/ikos-view.py.in script/ikos-view @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-view" DESTINATION bin)

configure_file(script/ikos-scan.py.in script/ikos-scan @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-scan" DESTINATION bin)

configure_file(script/ikos-scan-cc.py.in script/ikos-scan-cc @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-scan-cc" DESTINATION bin)

configure_file(script/ikos-scan-c++.py.in script/ikos-scan-c++ @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-scan-c++" DESTINATION bin)

configure_file(script/ikos-scan-extract.py.in script/ikos-scan-extract @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/script/ikos-scan-extract" DESTINATION bin)

#
# Regression tests
#

enable_testing()
add_custom_target(build-analyzer-tests)
add_subdirectory(test/regression EXCLUDE_FROM_ALL)

#
# Doxygen
#

find_package(Doxygen)
if (DOXYGEN_FOUND)
  configure_file(doc/doxygen/Doxyfile.in doc/Doxyfile @ONLY)
  add_custom_target(doxygen-analyzer
    ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating IKOS Analyzer API documentation with Doxygen" VERBATIM
  )
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/html" DESTINATION doc/analyzer OPTIONAL)
endif()

#
# If it's the top level CMakeLists.txt, Add some aliases
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS build-analyzer-tests)
  add_custom_target(doc DEPENDS doxygen-analyzer)
endif()
