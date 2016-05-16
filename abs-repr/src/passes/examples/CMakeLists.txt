add_library(hello SHARED hello.cpp)
target_link_libraries(hello arbos-api)
install(TARGETS hello DESTINATION lib OPTIONAL)

add_library(options SHARED options.cpp)
target_link_libraries(options arbos-api)
install(TARGETS options DESTINATION lib OPTIONAL)

add_library(visitor SHARED visitor.cpp)
target_link_libraries(visitor arbos-api)
install(TARGETS visitor DESTINATION lib OPTIONAL)

add_custom_target(example-passes DEPENDS hello options visitor)
