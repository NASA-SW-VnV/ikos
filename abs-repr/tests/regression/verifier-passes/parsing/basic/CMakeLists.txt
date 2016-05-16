add_library(test-parsing-basic SHARED test-basic-parsing.cpp)

target_link_libraries(test-parsing-basic arbos-api)

install(TARGETS test-parsing-basic DESTINATION lib OPTIONAL)
