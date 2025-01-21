cmake_minimum_required(VERSION 3.10)
project(tbt_recovery)

option(BUILD_STATIC_LIBS ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)
# Find required packages
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)


# Add library target
add_library(tbt_recovery_lib STATIC
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
    fmt::fmt
    spdlog::spdlog
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
