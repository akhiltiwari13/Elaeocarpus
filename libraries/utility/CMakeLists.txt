set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sources/exceptionprocessor)

add_library(elaeo-utils STATIC ${SOURCES})
target_include_directories(elaeo-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
