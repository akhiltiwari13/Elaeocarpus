#! /usr/bin/bash

# Set noninteractive mode to suppress prompts
export DEBIAN_FRONTEND=noninteractive

# Update package lists
sudo apt-get update

# Install base dependencies
sudo apt-get install -y \
    software-properties-common \
    build-essential \
    wget \
    git \
    python3 \
    pkg-config \
    ninja-build \
    libssl-dev \
    libpcre3-dev \
    zlib1g-dev \
    gdb \
    valgrind \
    doxygen

# Install GCC-7 (Note: GCC-7 is not available in Ubuntu 22.04 repositories)
# Attempt to install GCC-7 from PPA (may not work)
# sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
# sudo apt-get update
#
# # Check if GCC-7 is available
# if apt-cache policy gcc-7 | grep -q 'Candidate'; then
#     sudo apt-get install -y gcc-7 g++-7
#     # Set gcc-7 as default
#     sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60
#     sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 60
#     sudo update-alternatives --set gcc /usr/bin/gcc-7
#     sudo update-alternatives --set g++ /usr/bin/g++-7
# else
#     echo "GCC-7 is not available in the repositories for Ubuntu 22.04."
#     echo "Proceeding with the default GCC version."
# fi
#
# # Install CMake 3.11.0 (if required by your project)
# # Remove existing CMake if any
# sudo apt-get remove -y cmake
#
# # Download and install CMake 3.11.0
# wget https://github.com/Kitware/CMake/releases/download/v3.11.0/cmake-3.11.0-Linux-x86_64.sh -q -O /tmp/cmake-install.sh
# chmod +x /tmp/cmake-install.sh
# sudo mkdir /opt/cmake
# sudo /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake
# sudo ln -s /opt/cmake/bin/* /usr/local/bin/
# rm /tmp/cmake-install.sh
#
# # Install Boost 1.55.0 for bcp tool
# cd /tmp
# wget https://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz
# tar xf boost_1_55_0.tar.gz
# cd boost_1_55_0
# ./bootstrap.sh
# ./b2 tools/bcp
# sudo cp dist/bin/bcp /usr/local/bin/
# cd ..
# rm -rf boost_1_55_0*
#
# # Download Boost 1.63.0 and use bcp to extract required headers
# cd /tmp
# wget https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
# tar xf boost_1_63_0.tar.gz
# sudo mkdir -p /usr/local/include/boost
# # Use bcp to extract required headers
# bcp --boost=/tmp/boost_1_63_0 \
#    variant.hpp \
#    thread.hpp \
#    program_options.hpp \
#    system/error_code.hpp \
#    filesystem.hpp \
#    /usr/local/include
# cd ..
# rm -rf boost_1_63_0*

# Set working directory (adjust the path to your project's directory)
# cd /platform

# Make sure build.sh is executable
# chmod +x build.sh

# Run the project's build script
# ./build.sh Release
