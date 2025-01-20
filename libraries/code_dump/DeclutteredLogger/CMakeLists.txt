cmake_minimum_required(VERSION 3.10)

add_library(DeclutteredLogger log.cpp setaffinity.cpp)
target_link_libraries(DeclutteredLogger PUBLIC pthread)
target_include_directories(DeclutteredLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
