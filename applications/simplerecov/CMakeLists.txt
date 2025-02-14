option(BUILD_STATIC_LIBS ON)

# Use static linking for stdlib
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Add executable target
add_executable(simple_recovery
  main.cpp
)

# Ensure executable includes headers
target_include_directories(simple_recovery
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link all necessary dependencies
target_link_libraries(simple_recovery
  PRIVATE
    fmt::fmt
    spdlog::spdlog
)

# Remove static linking flags to avoid potential issues
set_target_properties(simple_recovery PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
