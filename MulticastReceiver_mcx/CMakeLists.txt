cmake_minimum_required(VERSION 3.21)
project(MCXReceiver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as static library" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "Do not build spdlog examples" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Do not build spdlog tests" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use internal fmt library" FORCE)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        yaml-cpp-0.7.0
)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static library" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Do not build yaml-cpp tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Do not build yaml-cpp tools" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

add_executable(mcx_receiver 
    src/mcx_mcast_receiver.cpp
    src/mcast_channel.cpp
)

target_include_directories(mcx_receiver PUBLIC ${CMAKE_SOURCE_DIR}/inc)

target_link_libraries(mcx_receiver
    PRIVATE
        spdlog::spdlog
        yaml-cpp
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cfg)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

add_custom_command(
    TARGET mcx_receiver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/cfg/mcx_mcast_cfg.yaml
        ${CMAKE_BINARY_DIR}/cfg/mcx_mcast_cfg.yaml
    COMMENT "Copying configuration file to build directory"
)

add_custom_command(
    TARGET mcx_receiver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/setup.sh
        ${CMAKE_BINARY_DIR}/setup.sh
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/setup.sh
    COMMENT "Copying and making setup script executable"
)

install(TARGETS mcx_receiver RUNTIME DESTINATION bin)
install(FILES cfg/mcx_mcast_cfg.yaml DESTINATION etc/mcx)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
