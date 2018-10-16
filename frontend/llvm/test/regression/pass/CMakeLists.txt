# Dependencies to run the tests
add_dependencies(build-frontend-llvm-tests ikos-pp)

# Try to find FileCheck
find_program(LLVM_FILE_CHECK
  CACHE NAMES FileCheck
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "FileCheck executable")
if (NOT LLVM_FILE_CHECK)
  message(WARNING "Could NOT find FileCheck. Tests for ikos-pp are disabled.")
endif()

function(add_pass_test test_name test_directory)
  if (NOT LLVM_FILE_CHECK)
    return() # Skip the test
  endif()

  add_test(NAME "pass-${test_name}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_directory}"
           COMMAND runtest
             --ikos-pp "$<TARGET_FILE:ikos-pp>"
             --file-check "${LLVM_FILE_CHECK}")
endfunction()

add_pass_test(lower-cst-expr lower_cst_expr)
add_pass_test(lower-select lower_select)
add_pass_test(remove-printf-calls remove_printf_calls)
add_pass_test(remove-unreachable-blocks remove_unreachable_blocks)
