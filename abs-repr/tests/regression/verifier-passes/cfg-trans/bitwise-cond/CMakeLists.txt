add_library(test-cfg-trans-bitwise-cond SHARED test-cfg-trans-bitwise-cond.cpp)

target_link_libraries(test-cfg-trans-bitwise-cond arbos-api)

install(TARGETS test-cfg-trans-bitwise-cond DESTINATION lib OPTIONAL)
