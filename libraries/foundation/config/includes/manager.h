#pragma once

#include <typeindex> //@TODO: investigate why there isn't a underscore in this header file's name.
#include <type_traits>
#include <unordered_map>
#include <any>
#include <string>
#include <stdexcept>

namespace elaeo::foundation::config {

class Manager {
public:
  template <typename T>
  static void load(const std::string& path) {
    T::load(path); // Static dispatch via CRTP
    get_store().emplace(typeid(T), T{});
  }

  template <typename T>
  static const T& get() {
    auto it = get_store().find(typeid(T));
    if (it == get_store().end()) throw std::runtime_error("Config not loaded");
    return std::any_cast<const T&>(it->second);
  }

private:
  static std::unordered_map<std::type_index, std::any>& get_store() {
    static std::unordered_map<std::type_index, std::any> store;
    return store;
  }
};

} // namespace elaeo::foundation::config
