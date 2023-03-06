#*******************************************************************************
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2019 United States Government as represented by the
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

project(ikos-frontend-llvm)

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
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

set(CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation")
if (CUSTOM_BOOST_ROOT)
  set(BOOST_ROOT "${CUSTOM_BOOST_ROOT}")
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

find_package(Boost 1.55.0 REQUIRED
             COMPONENTS filesystem system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(SYSTEM ${GMP_INCLUDE_DIR})
include_directories(SYSTEM ${GMPXX_INCLUDE_DIR})

find_package(Core REQUIRED)
include_directories(${CORE_INCLUDE_DIR})

find_package(AR REQUIRED)
include_directories(${AR_INCLUDE_DIR})

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
  add_compiler_flag(OPTIONAL "WNO_EXIT_TIME_DESTRUCTORS" "-Wno-exit-time-destructors")
  add_compiler_flag(OPTIONAL "WNO_GLOBAL_CONSTRUCTORS" "-Wno-global-constructors")
  add_compiler_flag(OPTIONAL "WNO_WEAK_VTABLES" "-Wno-weak-vtables")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compiler_flag(REQUIRED "WALL" "-Wall")
  add_compiler_flag(REQUIRED "WEXTRA" "-Wextra")
endif()

#
# Targets
#

include_directories(include)

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hpp"
)

# ikos-pp passes
add_library(ikos-pp-lib
  src/pass/initialize.cpp
  src/pass/lower_cst_expr.cpp
  src/pass/lower_select.cpp
  src/pass/mark_internal_inline.cpp
  src/pass/name_values.cpp
  src/pass/remove_printf_calls.cpp
  src/pass/remove_unreachable_blocks.cpp
)
if (IKOS_LINK_LLVM_DYLIB)
  set(IKOS_PP_LIB_LLVM_LIBS "LLVM")
else()
  llvm_map_components_to_libnames(IKOS_PP_LIB_LLVM_LIBS
    core
    support
    transformutils
  )
endif()
target_link_libraries(ikos-pp-lib ${IKOS_PP_LIB_LLVM_LIBS})
set_target_properties(ikos-pp-lib PROPERTIES OUTPUT_NAME ikos-pp)
install(TARGETS ikos-pp-lib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

# ikos-pp binary
add_executable(ikos-pp src/ikos_pp.cpp)
if (IKOS_LINK_LLVM_DYLIB)
  set(IKOS_PP_BIN_LLVM_LIBS "LLVM")
else()
  llvm_map_components_to_libnames(IKOS_PP_BIN_LLVM_LIBS
    aggressiveinstcombine
    analysis
    asmparser
    bitreader
    bitwriter
    codegen
    core
    coroutines
    instcombine
    instrumentation
    ipo
    irreader
    mc
    objcarcopts
    scalaropts
    support
    target
    transformutils
    vectorize
  )
endif()
target_link_libraries(ikos-pp
  ikos-pp-lib
  ${IKOS_PP_BIN_LLVM_LIBS}
  ${Boost_LIBRARIES}
)
install(TARGETS ikos-pp RUNTIME DESTINATION bin)

# llvm-to-ar lib
add_library(ikos-llvm-to-ar
  src/import/bundle.cpp
  src/import/constant.cpp
  src/import/data_layout.cpp
  src/import/exception.cpp
  src/import/function.cpp
  src/import/importer.cpp
  src/import/library_function.cpp
  src/import/source_location.cpp
  src/import/type.cpp
)
if (IKOS_LINK_LLVM_DYLIB)
  set(IKOS_LLVM_TO_AR_LLVM_LIBS "LLVM")
else()
  llvm_map_components_to_libnames(IKOS_LLVM_TO_AR_LLVM_LIBS
    core
    support
    transformutils
  )
endif()
target_link_libraries(ikos-llvm-to-ar
  ${IKOS_LLVM_TO_AR_LLVM_LIBS}
  ${AR_LIB}
  ${Boost_LIBRARIES}
  ${GMP_LIB}
  ${GMPXX_LIB}
)
install(TARGETS ikos-llvm-to-ar
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

# ikos-import binary
add_executable(ikos-import src/ikos_import.cpp)
if (IKOS_LINK_LLVM_DYLIB)
  set(IKOS_IMPORT_LLVM_LIBS "LLVM")
else()
  llvm_map_components_to_libnames(IKOS_IMPORT_LLVM_LIBS
    core
    ipo
    irreader
    support
    transformutils
  )
endif()
target_link_libraries(ikos-import
  ikos-llvm-to-ar
  ${IKOS_IMPORT_LLVM_LIBS}
  ${AR_LIB}
  ${Boost_LIBRARIES}
  ${GMP_LIB}
  ${GMPXX_LIB}
)
install(TARGETS ikos-import RUNTIME DESTINATION bin)

#
# Regression tests
#

enable_testing()
add_custom_target(build-frontend-llvm-tests)
add_subdirectory(test/regression/pass EXCLUDE_FROM_ALL)
add_subdirectory(test/regression/import EXCLUDE_FROM_ALL)

#
# If it's the top level CMakeLists.txt, Add some aliases
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS build-frontend-llvm-tests)
endif()
