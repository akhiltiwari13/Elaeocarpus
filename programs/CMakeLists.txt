cmake_minimum_required(VERSION 3.10)

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/libraries/DeclutteredLogger
    ${CMAKE_SOURCE_DIR}/libraries/DeclutteredConfigReader
    ${CMAKE_SOURCE_DIR}/libraries/DeclutteredSHMContainer
    ${CMAKE_SOURCE_DIR}/libraries/AssortedUtilities
    )

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE DeclutteredLogger DeclutteredConfigReader AssortedUtilities )
target_include_directories(sample PRIVATE ${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(shm_demo shm_demo.cpp)
target_link_libraries(shm_demo PRIVATE shmstream)
target_include_directories(shm_demo PRIVATE ${INCLUDE_DIRS})
