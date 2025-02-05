option(BUILD_STATIC_LIBS ON)

# Use static linking for stdlib
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Find required packages
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(asio REQUIRED)

# Add library target
add_library(asio_tbt_recovery_lib STATIC
  sources/tbt_recovery_client.cpp
)

target_include_directories(asio_tbt_recovery_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
  PRIVATE
    ${YAML_CPP_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR}
    ${asio_INCLUDE_DIR}
)

target_link_libraries(asio_tbt_recovery_lib
  PUBLIC
    yaml-cpp
    fmt::fmt
    spdlog::spdlog
    asio::asio
)

# Add executable target
add_executable(asio_tbt_recovery
  sources/main.cpp
  sources/tbt_recovery_client.cpp
)

# Ensure executable includes headers
target_include_directories(asio_tbt_recovery
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

# Link all necessary dependencies
target_link_libraries(asio_tbt_recovery
  PRIVATE
    asio_tbt_recovery_lib
    yaml-cpp
    fmt::fmt
    spdlog::spdlog
    asio::asio
)

# Installation
install(TARGETS asio_tbt_recovery_lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(TARGETS asio_tbt_recovery
  RUNTIME DESTINATION bin
)

install(FILES
  includes/tbt_recovery_client.h
  DESTINATION includes/asio_tbt_recovery
)

install(FILES
  config/tbt_recovery.yaml
  DESTINATION etc/tbt_recovery
)

# Remove static linking flags to avoid potential issues
set_target_properties(asio_tbt_recovery PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
