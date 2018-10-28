# Dependencies to run the tests
add_dependencies(build-analyzer-tests ikos-analyzer)

function(add_analysis_test test_name test_directory)
  add_test(NAME "analysis-${test_name}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_directory}"
           COMMAND ${PYTHON_EXECUTABLE} runtest
             --clang "${CLANG_EXECUTABLE}"
             --ikos-pp "${FRONTEND_LLVM_IKOS_PP_EXECUTABLE}"
             --ikos-analyzer "$<TARGET_FILE:ikos-analyzer>")
endfunction()

add_analysis_test(buffer-overflow boa)
add_analysis_test(division-by-zero dbz)
add_analysis_test(memory mem)
add_analysis_test(null-pointer-dereference null)
add_analysis_test(assert-prover prover)
add_analysis_test(uninitialized-variable uva)
add_analysis_test(unaligned-pointer upa)
add_analysis_test(signed-int-overflow sio)
add_analysis_test(unsigned-int-overflow uio)
add_analysis_test(shift-count shc)
add_analysis_test(pointer-overflow poa)
add_analysis_test(pointer-compare pcmp)
add_analysis_test(function-call fca)
add_analysis_test(double-free dfa)
add_analysis_test(soundness sound)
