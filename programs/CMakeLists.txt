cmake_minimum_required(VERSION 3.10)

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/libraries/DeclutteredLogger
    ${CMAKE_SOURCE_DIR}/libraries/DeclutteredConfigReader
    )

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE DeclutteredLogger DeclutteredConfigReader)
target_include_directories(sample PRIVATE ${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
