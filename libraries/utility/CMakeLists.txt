set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sources/exceptionprocessor)

add_library(elaeo-util STATIC ${SOURCES})
target_include_directories(elaeo-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
