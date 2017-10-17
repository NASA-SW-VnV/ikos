add_library(test-cfg-trans-llvm-phi-4 SHARED test-cfg-trans-llvm-phi-4.cpp)

target_link_libraries(test-cfg-trans-llvm-phi-4 arbos-api)

install(TARGETS test-cfg-trans-llvm-phi-4 DESTINATION lib OPTIONAL)
