add_library(test-cfg-trans-llvm-phi-1 SHARED test-cfg-trans-llvm-phi-1.cpp)

target_link_libraries(test-cfg-trans-llvm-phi-1 arbos-api)

install(TARGETS test-cfg-trans-llvm-phi-1 DESTINATION lib OPTIONAL)
