option(BUILD_STATIC_LIBS ON)
# Use static linking for stdlib
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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

# Force static linking for all executables
set_target_properties(tbt_recovery PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
