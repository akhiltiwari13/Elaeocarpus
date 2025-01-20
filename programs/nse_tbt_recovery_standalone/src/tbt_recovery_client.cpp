#include "tbt_recovery_client.h"
#include <yaml-cpp/yaml.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "nse_tbt_packet_structure.h"
#include <fmt/format.h>

namespace acce {
namespace recovery {
class TbtRecoveryClient::Logger {
public:
    explicit Logger(const std::string& log_file) {
        m_logger = spdlog::daily_logger_mt("tbt_recovery", log_file);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        m_logger->set_level(spdlog::level::debug);
        m_logger->flush_on(spdlog::level::info);
    }
    
    template<typename... Args>
    void info(const char* fmt, const Args&... args) {
        m_logger->info(fmt, args...);
    }

    template<typename... Args>
    void debug(const char* fmt, const Args&... args) {
        m_logger->debug(fmt, args...);
    }

    template<typename... Args>
    void error(const char* fmt, const Args&... args) {
        m_logger->error(fmt, args...);
    }

    void hexdump(const void* data, size_t size, const char* prefix = "") {
        auto* bytes = static_cast<const uint8_t*>(data);
        std::string hex_line;
        std::string ascii_line;
        
        for (size_t i = 0; i < size; i++) {
            if (i % 16 == 0) {
                if (!hex_line.empty()) {
                    m_logger->info("{}{:<48} {}", prefix, hex_line, ascii_line);
                    hex_line.clear();
                    ascii_line.clear();
                }
                hex_line = fmt::format("{:04x}: ", i);
            }
            
            hex_line += fmt::format("{:02x} ", bytes[i]);
            ascii_line += (bytes[i] >= 32 && bytes[i] <= 126) ? static_cast<char>(bytes[i]) : '.';
        }
        
        if (!hex_line.empty()) {
            m_logger->info("{}{:<48} {}", prefix, hex_line, ascii_line);
        }
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

class TbtRecoveryClient::TcpConnection {
public:
  TcpConnection() : m_socket(-1) {}
  
  ~TcpConnection() {
    if (m_socket >= 0) {
      close(m_socket);
    }
  }

  bool connect(const std::string& ip, uint16_t port, uint32_t timeout_ms) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) return false;

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    // Set timeout
    struct timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    return ::connect(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0;
  }

  bool send(const void* data, size_t size) {
    return ::send(m_socket, data, size, 0) == size;
  }

  ssize_t receive(void* buffer, size_t size) {
    return ::recv(m_socket, buffer, size, 0);
  }

private:
  int m_socket;
};

TbtRecoveryClient::TbtRecoveryClient(const std::string& config_file) 
  : m_config_file(config_file) {}

TbtRecoveryClient::~TbtRecoveryClient() = default;

bool TbtRecoveryClient::initialize() {
  return loadConfig(m_config_file);
}

bool TbtRecoveryClient::loadConfig(const std::string& config_file) {
  try {
    YAML::Node config = YAML::LoadFile(config_file);

    // Initialize logger
    m_logger = std::make_unique<Logger>(config["log"]["file"].as<std::string>());

    // Load server configs
    auto servers = config["servers"];
    
    // Map segment strings to enum
    static const std::unordered_map<std::string, Segment> segment_map = {
      {"CM", Segment::CM},
      {"FO", Segment::FO}, 
      {"CD", Segment::CD},
      {"CO", Segment::CO}
    };

    // Load config for each segment
    for (const auto& server : servers) {
      auto segment_str = server.first.as<std::string>();
      auto segment = segment_map.at(segment_str);
      
      RecoveryConfig cfg{
        server.second["ip"].as<std::string>(),
        server.second["port"].as<uint16_t>(),
        config["recovery"]["timeout_ms"].as<uint32_t>(),
        config["recovery"]["max_retries"].as<uint32_t>(),
        config["recovery"]["buffer_size"].as<uint32_t>()
      };
      
      m_configs[segment] = cfg;
    }

    return true;
  }
  catch(const std::exception& e) {
    if(m_logger) {
      m_logger->error("Failed to load config: {}", e.what());
    }
    return false;
  }
}
bool TbtRecoveryClient::requestRecovery(const RecoveryRequest& request) {
    // Get config for requested segment
    auto it = m_configs.find(request.segment);
    if (it == m_configs.end()) {
        m_logger->error("Invalid segment requested");
        return false;
    }
    const auto& config = it->second;

    // Connect to recovery server
    m_connection = std::make_unique<TcpConnection>();
    if (!m_connection->connect(config.server_ip, config.server_port, config.timeout_ms)) {
        m_logger->error("Failed to connect to recovery server {}:{}", 
                        config.server_ip, config.server_port);
        return false;
    }

    // Set the end sequence number
    m_end_seq = request.end_seq;

    // Send recovery request
    RecoveryRequestPacket req_packet{};
    req_packet.msg_type = 'R';
    req_packet.stream_id = request.stream_id;
    req_packet.start_seq = request.start_seq;
    req_packet.end_seq = request.end_seq;

    if (!sendRequest(req_packet)) {
        m_logger->error("Failed to send recovery request");
        return false;
    }

    // Process responses
    return processResponses();
}

bool TbtRecoveryClient::sendRequest(const RecoveryRequestPacket& request) {
  return m_connection->send(&request, sizeof(request));
}

bool TbtRecoveryClient::processResponses() 
{
    bool firstYReceived = false;

    while (true) 
    {
        // (1) Read the 9-byte StreamHeader
        StreamHeader hdr{};
        ssize_t bytes_read = m_connection->receive(&hdr, sizeof(hdr));
        if (bytes_read <= 0) {
            // connection closed or error => done
            break;
        }
        if (bytes_read != static_cast<ssize_t>(sizeof(hdr))) {
            m_logger->error("Incomplete TBT header read: got {} vs 9 bytes", bytes_read);
            return false;
        }

        // (2) Convert from network to host endianness
        // fixEndianness(hdr); //@todo check if this works?

        // (3) If this is a control message 'Y' ...
        if (hdr.message_type == 'Y') 
        {
            // If it's our first 'Y', interpret it as the "Recovery Response" or an ACK
            if (!firstYReceived) {
                m_logger->info("Received first control message 'Y' => reading Recovery Response payload...");
                
                // The doc usually says there's 1 extra byte for status or so.  
                // But if hdr.msg_len > 0, read that many bytes fully:
                if (hdr.msg_len > 0) {
                    std::vector<uint8_t> ctrlPayload(hdr.msg_len);
                    size_t total_read = 0;
                    while (total_read < hdr.msg_len) {
                        ssize_t ret = m_connection->receive(ctrlPayload.data() + total_read, hdr.msg_len - total_read);
                        if (ret <= 0) {
                            m_logger->error("Could not read full 'Y' payload: got {} vs {}", total_read, hdr.msg_len);
                            return false;
                        }
                        total_read += ret;
                    }

                    // Now parse ctrlPayload if needed, e.g. check if it's 'S' or 'E'
                    // For example, if the doc says the first byte is 'S'/'E':
                    if (!ctrlPayload.empty()) {
                        char status = static_cast<char>(ctrlPayload[0]);
                        if (status == 'E') {
                            m_logger->error("Recovery request denied (status='E'). Closing connection.");
                            m_connection.reset();
                            return false;
                        } else if (status == 'S') {
                            m_logger->info("Recovery request accepted (status='S'). Will read TBT next...");
                            firstYReceived = true; // now we read TBT in subsequent steps
                        } else {
                            m_logger->error("Unknown recovery status '{}', closing.", status);
                            m_connection.reset();
                            return false;
                        }
                    }
                    else {
                        // Possibly no status byte? Then just assume success?
                        m_logger->info("No 'Y' payload. Assuming success, continuing TBT...");
                        firstYReceived = true;
                    }
                }
                else {
                    // No payload for the first Y => assume success
                    m_logger->info("No payload with first 'Y'. Assuming success...");
                    firstYReceived = true;
                }

                // Continue loop to read next message (TBT or second Y)
                continue;
            }
            else {
                // We already got the first 'Y' => This second 'Y' indicates end-of-data
                m_logger->info("Received second control message 'Y'. Closing connection.");
                m_connection.reset();
                return true;
            }
        } // end if (hdr.message_type == 'Y')

        // If we get here, it's NOT 'Y', so it's a TBT message => read payload
        std::vector<uint8_t> payload(hdr.msg_len);
        size_t total_bytes_read = 0;
        while (total_bytes_read < hdr.msg_len) {
            ssize_t ret = m_connection->receive(payload.data() + total_bytes_read,
                                                hdr.msg_len - total_bytes_read);
            if (ret <= 0) {
                m_logger->error("Incomplete TBT packet received: got {} vs {}", 
                                total_bytes_read, hdr.msg_len);
                return false;
            }
            total_bytes_read += ret;
        }

        m_logger->info("Received TBT packet: seq={} size={}", hdr.seq_no, hdr.msg_len);
        m_logger->hexdump(payload.data(), hdr.msg_len, "  ");

        // Process the TBT data
        processTbtMessage(hdr, payload.data(), hdr.msg_len);

        // Fire user callback if set
        if (m_callback) {
            m_callback(hdr.seq_no, payload);
        }

        // Optionally, if you track m_end_seq, check if (hdr.seq_no >= m_end_seq)
        if (hdr.seq_no >= m_end_seq) {
            m_logger->info("Reached end of requested sequence range. Closing connection.");
            m_connection.reset();
            return true;
        }
    }

    // If we break out of while(true), close too
    m_connection.reset();
    return true;
}

// ---------------------------------------------------------------------

void TbtRecoveryClient::processTbtMessage(
    const StreamHeader& tbtHeader,
    const uint8_t* payload,
    size_t length)
{
    // We already called fixEndianness(hdr) in processResponses(),
    // so tbtHeader is in host-endian. But the memory for the
    // *rest* of the message is still big-endian. We'll fix it in-place.

    MessageType msgType = static_cast<MessageType>(tbtHeader.message_type);

    switch (msgType)
    {
    case MessageType::NewOrder:
    case MessageType::ModifyOrder:
    case MessageType::CancelOrder:
        {
            if (length < sizeof(OrderMessage)) {
                m_logger->error("Payload too small ({} bytes) for OrderMessage", length);
                return;
            }
            auto* orderPtr = reinterpret_cast<OrderMessage*>(const_cast<uint8_t*>(payload));
            // fixEndianness(*orderPtr);

            m_logger->info(
                "Order: seq={} stream={} type={} token={} orderid={:.0f} price={} qty={}",
                orderPtr->header.seq_no,
                orderPtr->header.stream_id,
                static_cast<char>(orderPtr->header.message_type),
                orderPtr->token,
                orderPtr->order_id,
                orderPtr->price,
                orderPtr->quantity
            );
            break;
        }

    case MessageType::Trade:
        {
            if (length < sizeof(TradeMessage)) {
                m_logger->error("Payload too small ({} bytes) for TradeMessage", length);
                return;
            }
            auto* tradePtr = reinterpret_cast<TradeMessage*>(const_cast<uint8_t*>(payload));
            // fixEndianness(*tradePtr);

            m_logger->info(
                "Trade: seq={} stream={} token={} buyOrd={:.0f} sellOrd={:.0f} price={} qty={}",
                tradePtr->header.seq_no,
                tradePtr->header.stream_id,
                tradePtr->token,
                tradePtr->buy_order_id,
                tradePtr->sell_order_id,
                tradePtr->trade_price,
                tradePtr->trade_quantity
            );
            break;
        }

    case MessageType::PacketLoss:
        {
            if (length < sizeof(PacketLossPacket)) {
                m_logger->error("Payload too small ({} bytes) for PacketLossPacket", length);
                return;
            }
            auto* lossPtr = reinterpret_cast<PacketLossPacket*>(const_cast<uint8_t*>(payload));
            // fixEndianness(*lossPtr);

            m_logger->info(
                "PacketLoss: stream={} from={} to={}",
                lossPtr->header.stream_id,
                lossPtr->from_seq,
                lossPtr->to_seq
            );
            break;
        }

    case MessageType::HeartBeat:
        {
            // Just a heartbeat
            m_logger->debug("Heartbeat: seq={} stream={}",
                            tbtHeader.seq_no, tbtHeader.stream_id);
            break;
        }

    default:
        {
            m_logger->info("Unknown message type: {}",
                           static_cast<char>(tbtHeader.message_type));
            break;
        }
    }
}


} // namespace recovery
} // namespace acce
