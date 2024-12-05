#pragma once
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "mcx_md_structures.h"
#include <sstream>

class MCXDebugger {
public:
    static std::shared_ptr<spdlog::logger> setupLoggers() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            "logs/mcx_debug.log", 0, 0);

        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>("mcx_debug", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::debug);
        return logger;
    }

    static void logPacketHeader(const PacketHeader& header, const std::shared_ptr<spdlog::logger>& logger) {
        logger->debug("Packet Header: template_id={}, seq={}, appl_seq={}, segment={}", 
            header.header.template_id, 
            header.header.msg_seq_num,
            header.appl_seq_num,
            header.market_segment_id);
    }

    static void logSocketInfo(int sd, const std::string& multicast_group, 
                            int port, const std::string& interface_ip,
                            const std::shared_ptr<spdlog::logger>& logger) {
        logger->debug("Socket Info: fd={}, group={}, port={}, interface={}", 
            sd, multicast_group, port, interface_ip);

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        if (getsockname(sd, (struct sockaddr*)&addr, &len) == 0) {
            logger->debug("Bound address: {}:{}", 
                inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        }
    }

    static void hexDump(const void* data, size_t size, 
                       const std::shared_ptr<spdlog::logger>& logger) {
        const unsigned char* p = (const unsigned char*)data;
        std::stringstream ss;
        for (size_t i = 0; i < size; i += 16) {
            ss << fmt::format("{:04x}: ", i);
            for (size_t j = 0; j < 16; j++) {
                if (i + j < size) {
                    ss << fmt::format("{:02x} ", p[i + j]);
                } else {
                    ss << "   ";
                }
            }
            ss << "  ";
            for (size_t j = 0; j < 16 && i + j < size; j++) {
                ss << (char)(isprint(p[i + j]) ? p[i + j] : '.');
            }
            logger->debug(ss.str());
            ss.str("");
        }
    }
};
