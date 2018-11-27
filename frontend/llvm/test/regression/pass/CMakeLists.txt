# Dependencies to run the tests
add_dependencies(build-frontend-llvm-tests ikos-pp)

# Try to find bash
find_program(BASH_EXECUTABLE
  CACHE NAMES bash
  DOC "Path to bash binary")
if (NOT BASH_EXECUTABLE)
  message(WARNING "Could NOT find bash. Tests for ikos-pp are disabled.")
endif()

# Try to find FileCheck
find_program(LLVM_FILE_CHECK_EXECUTABLE
  CACHE NAMES FileCheck
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "Path to FileCheck binary")
if (NOT LLVM_FILE_CHECK_EXECUTABLE)
  message(WARNING "Could NOT find FileCheck. Tests for ikos-pp are disabled.")
endif()

function(add_pass_test test_name test_directory)
  if ((NOT LLVM_FILE_CHECK_EXECUTABLE) OR (NOT BASH_EXECUTABLE))
    return() # Skip the test
  endif()

  add_test(NAME "pass-${test_name}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_directory}"
           COMMAND ${BASH_EXECUTABLE} runtest
             --ikos-pp "$<TARGET_FILE:ikos-pp>"
             --file-check "${LLVM_FILE_CHECK_EXECUTABLE}")
endfunction()

add_pass_test(lower-cst-expr lower_cst_expr)
add_pass_test(lower-select lower_select)
add_pass_test(remove-printf-calls remove_printf_calls)
add_pass_test(remove-unreachable-blocks remove_unreachable_blocks)
