cmake_minimum_required(VERSION 3.10)

add_library(AssortedUtilities INTERFACE)
# target_link_libraries(DeclutteredLogger PUBLIC pthread)
target_include_directories(AssortedUtilities INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
