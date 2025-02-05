# Elaeocarpus
<sup> Trading Platform Implementation

## Architecture.
Elaeocarpus is an event driven system that has various modules interacting via an event bus.

### diagram @TODO

### building Elaeocarpus and the sdk
- uses conan and cmake to configure/build the process.


### use of conan
> Explanation of conan.txt
> This conanfile.txt provides a solid foundation for a modern C++20 project with all the requested features. It includes libraries for serialization, networking, event-driven architecture, logging, configuration parsing, and more. The compiler flags ensure that your code is optimized, clean, and free of warnings.

1. [requires]
This section lists all the libraries your project depends on. Here’s a breakdown of each library:

    capnproto: A high-performance serialization library. It’s great for serializing data structures efficiently.
    cppfix: A hypothetical library for fixing common C++ issues. If this doesn’t exist, you can remove it or replace it with a similar library.
    asio: A cross-platform C++ library for network and low-level I/O programming. It supports TCP, UDP, and multicast connections.
    libevent: An event-driven architecture library that provides a mechanism to execute a callback function when a specific event occurs.
    folly: Facebook’s open-source C++ library, which includes fast in-memory queues, string utilities, and more.
    spdlog: A fast and feature-rich logging library that supports both binary and text logging.
    fmt: A modern formatting library used by spdlog for string formatting.
    cpptoml: A library for parsing TOML configuration files.
    gtest: Google’s C++ testing framework. It’s optional but highly recommended for unit testing.

2. [generators]

This section specifies the build system generator. In this case, we’re using cmake to generate build files.
3. [options]

This section configures the build options for the libraries and the project itself:

    shared=False: Build static libraries by default.
    fPIC=True: Enable Position Independent Code (useful for shared libraries).
    build_type=Release: Build in release mode with optimizations enabled.
    compiler.cppstd=20: Use C++20 standard.
    compiler.libcxx=libstdc++11: Use the GNU standard C++ library (libstdc++11).
    compiler.warning_level=everything: Enable all compiler warnings.
    compiler.treat_warnings_as_errors=True: Treat all warnings as errors to enforce strict code quality.
    compiler.style_checks=strict: Enable strict style checks (if supported by the compiler).
    compiler.lto=True: Enable Link-Time Optimization (LTO) for better performance.
    compiler.debug=True: Include debug symbols even in release mode for better debugging.

4. [imports]

This section copies shared libraries (.dll, .so, .dylib) to the bin or lib directories in your project. This is useful for ensuring that all dependencies are available at runtime.
5. [build_requires]

This section specifies build-time dependencies. Here, we’re requiring a modern version of CMake (3.26.0) to ensure compatibility with the latest features.
Additional Features and Libraries

1. Unit Testing with Google Test (gtest)

    gtest: Google’s C++ testing framework is included for unit testing. You can write tests for your code to ensure correctness and prevent regressions.

2. Logging with spdlog

    spdlog: A fast and feature-rich logging library that supports both binary and text logging. It’s highly configurable and can log to files, consoles, or even remote servers.

3. Configuration Parsing with cpptoml

    cpptoml: A library for parsing TOML configuration files. TOML is a human-readable format for configuration files, and cpptoml makes it easy to read and write TOML files in C++.

4. Event-Driven Architecture with libevent

    libevent: This library provides a simple and efficient way to handle events in your application. It’s great for building event-driven systems like servers or GUI applications.

5. Fast In-Memory Queues with Folly

    folly: Facebook’s Folly library includes high-performance data structures like in-memory queues, which are useful for building high-throughput systems.

6. Network I/O with Asio

    asio: This library provides a powerful and flexible API for network programming. It supports both synchronous and asynchronous I/O, making it suitable for building high-performance network applications.

7. Serialization with Cap’n Proto

    capnproto: A high-performance serialization library that allows you to serialize and deserialize data structures efficiently. It’s particularly useful for communication between different components of a distributed system.

Compiler Flags and Optimizations

    C++20: The project is set to use the C++20 standard, which includes modern features like concepts, ranges, and coroutines.
    Warnings and Errors: All warnings are enabled and treated as errors to enforce strict code quality.
    Link-Time Optimization (LTO): LTO is enabled to optimize the final binary by allowing the compiler to optimize across translation units.
    Debug Symbols: Debug symbols are included even in release mode to facilitate debugging.

Conclusion

