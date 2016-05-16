add_library(test-cfg-trans-llvm-select SHARED test-cfg-trans-llvm-select.cpp)

target_link_libraries(test-cfg-trans-llvm-select arbos-api)

install(TARGETS test-cfg-trans-llvm-select DESTINATION lib OPTIONAL)
