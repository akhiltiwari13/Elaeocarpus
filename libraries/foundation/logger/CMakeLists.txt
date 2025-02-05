
# move these build options to a project wide dir.
option(BUILD_STATIC_LIBS ON)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Find required packages
find_package(spdlog REQUIRED)
find_package(CapnProto REQUIRED)
# find_package(fmt REQUIRED)
# find_package(asio REQUIRED)


# Add library target
add_library(elaeo-found-logger INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

target_include_directories(elaeo-found-logger
  # PUBLIC
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
  # PRIVATE
  # INTERFACE
    ${SPDLOG_INCLUDE_DIR}
    ${CAPNPROTO_INCLUDE_DIR}
)

target_link_libraries(elaeo-found-logger
  # PUBLIC
  INTERFACE
    # yaml-cpp
    # fmt::fmt
    spdlog::spdlog
    capnproto::CapnProto
    # asio::asio
)
# target_compile_definitions(elaeo-found-logger PRIVATE
#     ASIO_STANDALONE  # Add if using standalone ASIO
# )
# target_compile_definitions(elaeo-found-logger
#     PRIVATE
#     BOOST_ASIO_STANDALONE
#     BOOST_ASIO_HAS_STD_CHRONO
#     BOOST_ASIO_HAS_STD_ARRAY
#     BOOST_ASIO_HAS_CSTDINT
#     BOOST_ASIO_HAS_STD_SHARED_PTR
# )

# Add executable target
# add_executable(asio_tbt_recovery
#   sources/main.cpp
#   sources/tbt_recovery_client.cpp
# )

# target_link_libraries(asio_tbt_recovery
#   PRIVATE
#     asio_tbt_recovery_lib
# )
#
# # Installation
# install(TARGETS asio_tbt_recovery_lib
#   LIBRARY DESTINATION lib
#   ARCHIVE DESTINATION lib
# )
#
# install(TARGETS tbt_recovery
#   RUNTIME DESTINATION bin
# )
#
# install(FILES 
#   includes/tbt_recovery_client.h
#   DESTINATION includes/tbt_recovery
# )
#
# install(FILES
#   config/tbt_recovery.yaml
#   DESTINATION etc/tbt_recovery
# )
#
# # Force static linking for all executables
# set_target_properties(tbt_recovery PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
