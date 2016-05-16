add_executable(test_constants test_constants.cpp)
target_link_libraries(test_constants ${GMPXX_LIB} ${GMP_LIB})

add_executable(test_constant_domain test_constant_domain.cpp)
target_link_libraries(test_constant_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_constants RUNTIME DESTINATION bin)
install(TARGETS test_constant_domain RUNTIME DESTINATION bin)
