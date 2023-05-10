cmake_minimum_required(VERSION 3.8)

project(elaeocarpus VERSION 0.1 LANGUAGES CXX)

message(status "***pre-build*** ${PROJECT_SOURCE_DIR}")

if(UNIX)
    message(status "Building on Linux/Unix")
else(UNIX)
    message(status "Building on an unsupported/untested platform.")
endif(UNIX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")


add_subdirectory(libraries)
add_subdirectory(programs)
