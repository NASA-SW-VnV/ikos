add_executable(test_uninitialized test_uninitialized.cpp)
target_link_libraries (test_uninitialized ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_uninitialized RUNTIME DESTINATION bin)
