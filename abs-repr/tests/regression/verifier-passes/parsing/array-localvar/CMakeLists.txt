add_library(test-parsing-array-localvar-2 SHARED test-parsing-array-localvar-2.cpp)

target_link_libraries(test-parsing-array-localvar-2 arbos-api)

install(TARGETS test-parsing-array-localvar-2 DESTINATION lib OPTIONAL)
