cmake_minimum_required(VERSION 3.13)

project(thirdparty)

#add_subdirectory(fmt)

add_library(3pt INTERFACE)
add_dependencies(3pt fmt::fmt)

#target_link_libraries(3pt INTERFACE fmt::fmt)
