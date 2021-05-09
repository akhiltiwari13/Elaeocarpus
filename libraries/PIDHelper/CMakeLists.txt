cmake_minimum_required (VERSION 3.6)
include_directories(${CMAKE_SOURCE_DIR})

add_library( PIDHelper STATIC create_pid_file error_functions get_num region_locking )


