#pragma once
#include <string>

namespace elaeo::foundtion::config {

//using crtp for static polymorphism.
template <typename Derived>
struct Loader {
  // Compile-time interface for parsing
  static void load(const std::string& path) {
  // delegate loading to derived class
    Derived::load_impl(path); 
  }
};

} // namespace elaeo::foundation::config
