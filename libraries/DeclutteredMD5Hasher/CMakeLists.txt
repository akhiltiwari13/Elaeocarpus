cmake_minimum_required(VERSION 3.10)

add_library(DeclutteredMD5Hasher md5_helper.cpp)
target_link_libraries(DeclutteredMD5Hasher PUBLIC pthread)
target_include_directories(DeclutteredMD5Hasher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
