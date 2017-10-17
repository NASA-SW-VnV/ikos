add_library(test-parsing-src-lines-1 SHARED test-parsing-src-lines-1.cpp)

target_link_libraries(test-parsing-src-lines-1 arbos-api)

install(TARGETS test-parsing-src-lines-1 DESTINATION lib OPTIONAL)
