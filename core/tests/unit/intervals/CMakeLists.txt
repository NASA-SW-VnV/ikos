add_executable(test_intervals test_intervals.cpp)
target_link_libraries(test_intervals ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_intervals RUNTIME DESTINATION bin)
