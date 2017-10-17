add_executable(test_nullity test_nullity.cpp)
target_link_libraries (test_nullity ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_nullity RUNTIME DESTINATION bin)
