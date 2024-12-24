#! /bin/sh
# This script looks for various dependencies for a cmake/cpp project including various lib dependencies, headers etc.

# Search for find_package commands
rg -g 'CMakeLists.txt' 'find_package|find_library'

# Search for external library dependencies
rg -g 'CMakeLists.txt' 'lib.*-dev|lib.*\.so|\.a'

# Search for include directories that might indicate dependencies
rg -g 'CMakeLists.txt' 'include_directories|target_include_directories'

# Search for link libraries
rg -g 'CMakeLists.txt' 'target_link_libraries|link_libraries'

# Search for pkg-config usage
rg -g 'CMakeLists.txt' 'pkg_check_modules|pkg_search_module'

# Search for specific system headers in source files
rg -g '*.{h,hpp,cpp}' '^[[:space:]]*#include[[:space:]]*<[^>]+>'

# Search for boost libraries specifically
rg -g 'CMakeLists.txt' 'Boost::|boost'

# Search for common system library prefixes
rg -g 'CMakeLists.txt' 'lib(ssl|xml|z|png|jpeg|tiff|curl|crypto|pcre|dl)'

# Look for required components in find_package
rg -g 'CMakeLists.txt' 'COMPONENTS.*REQUIRED'

# Search for add_subdirectory commands to find internal dependencies
rg -g 'CMakeLists.txt' 'add_subdirectory'
