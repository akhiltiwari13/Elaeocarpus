#include "exceptionprocessor.h"
#include <memory>
#include <cxxabi.h> // For demangling type names

namespace elaeo::util {

namespace {
    // Helper function to demangle type names
    std::string demangle(const char* name) noexcept {
        int status = 0;
        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
        };
        return (status == 0) ? res.get() : name;
    }
}

void ExceptionProcessor::processException(const std::source_location& location) noexcept {
    try {
        throw; // Rethrow to process
    }
    catch (const std::ios_base::failure& e) {
        auto details = processCodeException(e);
        details.severity = ExceptionSeverity::Error;
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("I/O Exception at {}:{} in {}: {}\n",
                location.file_name(), location.line(), location.function_name(), e.what());
            std::cerr << std::format("Category: {}\nValue: {}\nMessage: {}\n",
                details.category, details.value, details.message);
        }
    }
    catch (const std::system_error& e) {
        auto details = processCodeException(e);
        details.severity = ExceptionSeverity::Critical;
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("System Error at {}:{}: {}\n",
                location.file_name(), location.line(), e.what());
        }
    }
    catch (const std::future_error& e) {
        auto details = processCodeException(e);
        details.severity = ExceptionSeverity::Error;
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("Future Error at {}:{}: {}\n",
                location.file_name(), location.line(), e.what());
        }
    }
    catch (const std::bad_alloc& e) {
        ExceptionDetails details{
            .category = "memory",
            .message = e.what(),
            .severity = ExceptionSeverity::Critical
        };
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("Memory Allocation Error at {}:{}: {}\n",
                location.file_name(), location.line(), e.what());
        }
    }
    catch (const std::exception& e) {
        ExceptionDetails details{
            .category = demangle(typeid(e).name()),
            .message = e.what(),
            .severity = ExceptionSeverity::Error
        };
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("Exception at {}:{}: {}\n",
                location.file_name(), location.line(), e.what());
        }
    }
    catch (...) {
        ExceptionDetails details{
            .category = "unknown",
            .message = "Unknown exception",
            .severity = ExceptionSeverity::Critical
        };
        
        if (errorHandler_) {
            errorHandler_(details, location);
        } else {
            std::cerr << std::format("Unknown Exception at {}:{}\n",
                location.file_name(), location.line());
        }
    }
}

} // namespace elaeo::util
