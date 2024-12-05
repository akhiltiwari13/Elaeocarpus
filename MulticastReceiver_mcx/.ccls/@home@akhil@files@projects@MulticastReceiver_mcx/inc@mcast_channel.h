#pragma once

#include <string>
#include <memory>
#include <system_error>
#include <string_view>
#include <spdlog/spdlog.h>

class MulticastChannel {
private:
    int sd_{-1};
    std::string multicast_group_;
    uint16_t multicast_port_;
    std::string local_interface_ip_;
    bool blocking_;
    int stream_id_;
    std::shared_ptr<spdlog::logger> logger_;

public:
  MulticastChannel(std::string_view multicast_group, 
               uint16_t port,
               std::string_view local_ip,
               bool blocking,
               int stream_id);
  ~MulticastChannel();
    int readData(void* databuf, int datalen);
    void start();
    void stop();
    [[nodiscard]] int streamId() const { return stream_id_; }
};

// @TODO:
// stringview vs string.
// shared_ptr vs unique_ptr<>'s const ref
