#include "mcast_channel.h"
#include "mcx_debug.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <spdlog/sinks/rotating_file_sink.h>

MulticastChannel::MulticastChannel(std::string_view multicast_group, 
                                 uint16_t port,
                                 std::string_view local_ip,
                                 bool blocking,
                                 int stream_id)
    : multicast_group_(multicast_group)
    , multicast_port_(port)
    , local_interface_ip_(local_ip)
    , blocking_(blocking)
    , stream_id_(stream_id)
{
    logger_ = spdlog::get("mcx_receiver");
    if (!logger_) {
        logger_ = spdlog::default_logger();
    }
}

MulticastChannel::~MulticastChannel() {
    stop();
}

void MulticastChannel::start() {
    logger_->info("Starting multicast channel with group: {}, port: {}", 
                 multicast_group_, multicast_port_);

    sd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd_ < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    int reuse = 1;
    if (setsockopt(sd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(sd_);
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    }

    if (!blocking_) {
        int flags = fcntl(sd_, F_GETFL, 0);
        fcntl(sd_, F_SETFL, flags | O_NONBLOCK);
    }

    struct sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(multicast_port_);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd_, reinterpret_cast<struct sockaddr*>(&local_addr), 
            sizeof(local_addr)) < 0) {
        close(sd_);
        throw std::runtime_error("Bind failed");
    }

    struct ip_mreq group{};
    group.imr_multiaddr.s_addr = inet_addr(multicast_group_.c_str());
    group.imr_interface.s_addr = inet_addr(local_interface_ip_.c_str());

    if (setsockopt(sd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        close(sd_);
        throw std::runtime_error("Failed to join multicast group");
    }

    logger_->info("Successfully joined multicast group");
}

void MulticastChannel::stop() {
    if (sd_ >= 0) {
        close(sd_);
        sd_ = -1;
        logger_->info("Multicast channel stopped");
    }
}

int MulticastChannel::readData(void* databuf, int datalen) {
    int bytes_read = read(sd_, databuf, datalen);
    if (bytes_read > 0) {
        MCXDebugger::hexDump(databuf, bytes_read, logger_);
    } else if (bytes_read < 0 && errno != EAGAIN) {
        logger_->error("Read error: {}", strerror(errno));
    }
    return bytes_read;
}
