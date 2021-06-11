/**
 * @author- akhil
 * @brief- templated function to get details about an exception
 **/

#include <exception>
#include <future>
#include <iostream>

template <typename T> void processCodeException(const T &e) {
    auto c = e.code();
    std::cerr << "- category: " << c.category().name() << std::endl;
    std::cerr << "- value: " << c.value() << std::endl;
    std::cerr << "- msg: " << c.message() << std::endl;
    std::cerr << "- def category: " << c.default_error_condition().category().name() << std::endl;
    std::cerr << "- def value: " << c.default_error_condition().value() << std::endl;
    std::cerr << "- def msg: " << c.default_error_condition().message() << std::endl;
}
