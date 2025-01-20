cmake_minimum_required(VERSION 3.19)
project(solace_recovery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)


# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/main.cpp
    src/solace_recovery.cpp
)

# Create executable
add_executable(solace_recovery ${SOURCES})

# Link against static libraries
target_link_libraries(solace_recovery
    ${PROJECT_SOURCE_DIR}/libs/libsolclient.a
    pthread
    rt
    dl
)

set_target_properties(solace_recovery PROPERTIES
    INSTALL_RPATH "$ORIGIN/libs"
    BUILD_WITH_INSTALL_RPATH TRUE
)
