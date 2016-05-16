add_library(test-cfg-trans-llvm-phi-2 SHARED test-cfg-trans-llvm-phi-2.cpp)

target_link_libraries(test-cfg-trans-llvm-phi-2 arbos-api)

install(TARGETS test-cfg-trans-llvm-phi-2 DESTINATION lib OPTIONAL)
