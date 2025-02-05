#pragma once
#include <spdlog/sinks/base_sink.h>
#include <capnp/serialize.h>

namespace sdk::logging {

template<typename Mutex>
class BinarySink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit BinarySink(const std::string& filename) {
        // Initialize Cap'n Proto file writer
        writer_ = kj::newDiskFile(filename)->append();
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        capnp::MallocMessageBuilder builder;
        auto log_entry = builder.initRoot<LogEntry>();
        log_entry.setTimestamp(msg.time.time_since_epoch().count());
        log_entry.setLevel(static_cast<uint8_t>(msg.level));
        log_entry.setMessage(msg.payload.data(), msg.payload.size());
        
        // Zero-copy write
        capnp::writeMessage(*writer_, builder);
    }

private:
    kj::Own<kj::AppendableFile> writer_;
};

} // namespace sdk::logging
