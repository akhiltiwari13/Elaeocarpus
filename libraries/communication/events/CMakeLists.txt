##TODO: add library version info modules

option(BUILD_STATIC_LIBS ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)
add_library(elaeo-comm-events STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sources/events.cpp)

# Specify compile options for the library
target_compile_options(elaeo-comm-events
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
install(TARGETS elaeo-comm-events
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Install the executable
install(TARGETS elaeo-comm-events
    RUNTIME DESTINATION bin
)

# Export the library for use by other projects
export(TARGETS elaeo-comm-events FILE elaeo-comm-events-targets.cmake
)
