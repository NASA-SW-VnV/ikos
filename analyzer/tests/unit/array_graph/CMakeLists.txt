add_executable(test_array_graph test_array_graph.cpp)
target_link_libraries(test_array_graph ${GMPXX_LIB} ${GMP_LIB} ${Boost_LIBRARIES})

install(TARGETS test_array_graph RUNTIME DESTINATION bin)
