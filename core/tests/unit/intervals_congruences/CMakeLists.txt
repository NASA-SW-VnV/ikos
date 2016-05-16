add_executable(test_intervals_congruences test_intervals_congruences.cpp)
target_link_libraries(test_intervals_congruences ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_intervals_congruences RUNTIME DESTINATION bin)
