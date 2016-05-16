enable_testing()
add_test(NAME llvm-phi-4-cfg-trans
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest)
