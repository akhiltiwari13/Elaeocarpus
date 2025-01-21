cmake_minimum_required(VERSION 3.10)
project(tbt_recovery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# Find required packages
# find_package(yaml-cpp REQUIRED)
# find_package(spdlog REQUIRED)

include(FetchContent)

# First declare and configure fmt since spdlog depends on it
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0  # Using a stable version
)
set(FMT_INSTALL ON CACHE BOOL "Generate installation target for fmt" FORCE)
set(FMT_TEST OFF CACHE BOOL "Do not build fmt tests" FORCE)

# Now declare spdlog with external fmt support
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as static library" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "Do not build spdlog examples" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Do not build spdlog tests" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external fmt library" FORCE)  # Changed to ON

FetchContent_MakeAvailable(fmt spdlog)
# FetchContent_Declare(
#   spdlog
#   GIT_REPOSITORY https://github.com/gabime/spdlog.git
#   GIT_TAG        v1.11.0
# )
#
# set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as static library" FORCE)
# set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "Do not build spdlog examples" FORCE)
# set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Do not build spdlog tests" FORCE)
# set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use internal fmt library" FORCE)
# FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        yaml-cpp-0.7.0
)

set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static library" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Do not build yaml-cpp tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Do not build yaml-cpp tools" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# Add library target
add_library(tbt_recovery_lib
  src/tbt_recovery_client.cpp
)

target_include_directories(tbt_recovery_lib
  PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
  PRIVATE
    ${YAML_CPP_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR}
)

target_link_libraries(tbt_recovery_lib
  PUBLIC
    yaml-cpp
    spdlog::spdlog
    fmt
)

# Add executable target
add_executable(tbt_recovery
  src/main.cpp
  src/tbt_recovery_client.cpp
)

target_link_libraries(tbt_recovery
  PRIVATE
    tbt_recovery_lib
)

# Installation
install(TARGETS tbt_recovery_lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(TARGETS tbt_recovery
  RUNTIME DESTINATION bin
)

install(FILES 
  include/tbt_recovery_client.h
  DESTINATION include/tbt_recovery
)

install(FILES
  config/tbt_recovery.yaml
  DESTINATION etc/tbt_recovery
)

# Testing setup if needed enable_testing()
# add_subdirectory(tests)

# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cfg)
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)
#
# add_custom_command(
#     TARGET mcx_receiver POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         ${CMAKE_SOURCE_DIR}/cfg/mcx_mcast_cfg.yaml
#         ${CMAKE_BINARY_DIR}/cfg/mcx_mcast_cfg.yaml
#     COMMENT "Copying configuration file to build directory"
# )
#
# add_custom_command(
#     TARGET mcx_receiver POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         ${CMAKE_SOURCE_DIR}/setup.sh
#         ${CMAKE_BINARY_DIR}/setup.sh
#     COMMAND chmod +x ${CMAKE_BINARY_DIR}/setup.sh
#     COMMENT "Copying and making setup script executable"
# )
#
# install(TARGETS mcx_receiver RUNTIME DESTINATION bin)
# install(FILES cfg/mcx_mcast_cfg.yaml DESTINATION etc/mcx)
#
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
