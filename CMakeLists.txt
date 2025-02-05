cmake_minimum_required(VERSION 3.21)

project(elaeocarpus VERSION 0.1 LANGUAGES CXX)

message(status "***elaeocarpus' pre-build*** ${PROJECT_SOURCE_DIR}")

if(UNIX)
    message(status "Building on Linux/Unix")
else(UNIX)
    message(status "Building on an unsupported/untested platform.")
endif(UNIX)

# Enable C++20 features and disable compiler-specific extensions
# This ensures portability and adherence to the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# conan2.x setup
# include(${CMAKE_BINARY_DIR}/build/build/Release/generators/conan_toolchain.cmake)

add_subdirectory(libraries)
add_subdirectory(applications)
# add_subdirectory(tests)
# add_subdirectory(scripts)
