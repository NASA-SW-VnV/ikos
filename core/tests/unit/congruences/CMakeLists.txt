add_executable(test_congruences test_congruences.cpp)
target_link_libraries(test_congruences ${GMPXX_LIB} ${GMP_LIB})

add_executable(test_congruence_domain test_congruence_domain.cpp)
target_link_libraries(test_congruence_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_congruences RUNTIME DESTINATION bin)
install(TARGETS test_congruence_domain RUNTIME DESTINATION bin)
