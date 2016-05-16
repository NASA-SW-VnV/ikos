add_library(test-cfg-trans-llvm-phi-3 SHARED test-cfg-trans-llvm-phi-3.cpp)

target_link_libraries(test-cfg-trans-llvm-phi-3 arbos-api)

install(TARGETS test-cfg-trans-llvm-phi-3 DESTINATION lib OPTIONAL)
