cmake_minimum_required(VERSION 3.10)

add_library(CommonUtils INTERFACE )
# target_link_libraries(CommonUtils )
target_include_directories(CommonUtils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
