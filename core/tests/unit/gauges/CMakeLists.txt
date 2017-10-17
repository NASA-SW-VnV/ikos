add_executable(test_gauges test_gauges.cpp)
target_link_libraries(test_gauges ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_gauges RUNTIME DESTINATION bin)
