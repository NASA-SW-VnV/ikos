# Dependencies to run the tests
add_dependencies(build-frontend-llvm-tests ikos-import)

# Try to find bash
find_program(BASH_EXECUTABLE
  CACHE NAMES bash
  DOC "Path to bash binary")
if (NOT BASH_EXECUTABLE)
  message(WARNING "Could NOT find bash. Tests for llvm-to-ar are disabled.")
endif()

# Try to find FileCheck
find_program(LLVM_FILE_CHECK_EXECUTABLE
  CACHE NAMES FileCheck
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "Path to FileCheck binary")
if (NOT LLVM_FILE_CHECK_EXECUTABLE)
  message(WARNING "Could NOT find FileCheck. Tests for llvm-to-ar are disabled.")
endif()

function(add_import_test test_name test_directory)
  if ((NOT LLVM_FILE_CHECK_EXECUTABLE) OR (NOT BASH_EXECUTABLE))
    return() # Skip the test
  endif()

  add_test(NAME "import-${test_name}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_directory}"
           COMMAND ${BASH_EXECUTABLE} runtest
             --ikos-import "$<TARGET_FILE:ikos-import>"
             --file-check "${LLVM_FILE_CHECK_EXECUTABLE}")
endfunction()

add_import_test(no-optimization no_optimization)
add_import_test(basic-optimization basic_optimization)
add_import_test(aggressive-optimization aggressive_optimization)
