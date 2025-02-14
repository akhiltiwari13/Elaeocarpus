#pragma once

#include <exception>
#include <future>
#include <iostream>
#include <source_location> // C++20
#include <string_view>
#include <format> // C++20

namespace elaeo::util {

// Exception severity levels
enum class ExceptionSeverity {
    Info,
    Warning,
    Error,
    Critical
};

// Exception details structure
struct ExceptionDetails {
    std::string_view category{""};
    int value{};
    std::string message{""};
    std::string_view defaultCategory{""};
    int defaultValue{};
    std::string defaultMessage{""};
    ExceptionSeverity severity{ExceptionSeverity::Error};
};

// Main exception processor class
class ExceptionProcessor {
public:
    // Process code-based exceptions (system_error, future_error, etc)
    template <typename T>
    static ExceptionDetails processCodeException(const T& e) noexcept {
        const auto& code = e.code();
        return ExceptionDetails{
            .category = code.category().name(),
            .value = code.value(),
            .message = code.message(),
            .defaultCategory = code.default_error_condition().category().name(),
            .defaultValue = code.default_error_condition().value(),
            .defaultMessage = code.default_error_condition().message()
        };
    }

    // Process any exception with source location
    static void processException(const std::source_location& location = std::source_location::current()) noexcept;

    // Set custom error handler
    using ErrorHandler = std::function<void(const ExceptionDetails&, const std::source_location&)>;
    static void setErrorHandler(ErrorHandler handler) noexcept {
        errorHandler_ = std::move(handler);
    }

private:
    static inline ErrorHandler errorHandler_; // Custom error handler
};

// Helper function for convenient usage
inline void processCurrentException(const std::source_location& location = std::source_location::current()) noexcept {
    ExceptionProcessor::processException(location);
}

} // namespace elaeo::util
