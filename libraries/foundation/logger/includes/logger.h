#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <memory>
#include <string>

namespace elaeo::foundation::logging {

class Logger {
public:
    // Initialize thread pool once (.5GB queue, 1 thread)
    // static void init(size_t queue_size = 1048576, size_t thread_count = 1);
    static void init(size_t queue_size = 524288, size_t thread_count = 1);

    // get/create named logger (no virtual functions)
    static Logger& get(const std::string& name = "default");

    // Fast templated logging (compile-time level check)
    template <typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        if constexpr (ENABLE_DEBUG) { // Compile-time flag
            logger_->debug(fmt, std::forward<Args>(args)...);
        }
    }

    // Add binary sink (future-proofing) for logging in critical components
    void add_binary_sink(std::unique_ptr<spdlog::sinks::sink> sink);

private:
    explicit Logger(const std::string& name);
    std::shared_ptr<spdlog::async_logger> logger_;
};

} // namespace elaeo::foundation::logging
