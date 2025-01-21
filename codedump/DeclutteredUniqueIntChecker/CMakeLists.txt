cmake_minimum_required(VERSION 3.10)

add_library(DeclutteredUniqueIntChecker UniqueIntChecker.cpp)
# target_link_libraries(DeclutteredUniqueIntChecker )
target_include_directories(DeclutteredUniqueIntChecker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
