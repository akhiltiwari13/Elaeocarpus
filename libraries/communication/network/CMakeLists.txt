##TODO: add library version info modules

find_package(asio REQUIRED)
option(BUILD_STATIC_LIBS ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)
add_library(elaeo-comm-network STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sources/network.cpp)

target_include_directories(elaeo-comm-network
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${ASIO_CPP_INCLUDE_DIR}
)
target_link_libraries(elaeo-comm-network PUBLIC  asio::asio)

# Specify compile options for the library
target_compile_options(elaeo-comm-network
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# Specify link libraries and dependencies if any
# For example, if you have dependencies like Boost or others, link them here
# target_link_libraries(elaeo-comm-events PRIVATE Boost::boost)

# # Install rules
# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/includes/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install the library
install(TARGETS elaeo-comm-network
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Install the executable
install(TARGETS elaeo-comm-network
    RUNTIME DESTINATION bin
)

# Export the library for use by other projects
export(TARGETS elaeo-comm-network FILE elaeo-comm-network-targets.cmake
)

add_executable(networktest ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp)
target_link_libraries(networktest PRIVATE elaeo-comm-network)
