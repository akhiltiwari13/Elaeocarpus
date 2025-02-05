#include "tbt_recovery_client.h"
#include "nse_tbt_packet_structure.h"
#include <arpa/inet.h>
#include <fmt/format.h>
#include <iostream>
#include <netinet/in.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

namespace acce {
namespace recovery {
class TbtRecoveryClient::Logger {
public:
  explicit Logger(const std::string &log_file) {
    m_logger = spdlog::daily_logger_mt("tbt_recovery", log_file);
    m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    m_logger->set_level(spdlog::level::debug);
    m_logger->flush_on(spdlog::level::info);
  }

  // template<typename... Args>
  // void info(const char* fmt, const Args&... args) {
  //     m_logger->info(fmt, args...);
  // }
  //
  // template<typename... Args>
  // void debug(const char* fmt, const Args&... args) {
  //     m_logger->debug(fmt, args...);
  // }
  //
  // template<typename... Args>
  // void error(const char* fmt, const Args&... args) {
  //     m_logger->error(fmt, args...);
  // }
  //
  // template<typename... Args>
  // void warn(const char* fmt, const Args&... args) {
  //     m_logger->warn(fmt, args...);
  // }

  template <typename... Args>
  void info(fmt::format_string<Args...> fmt, Args &&...args) {
    m_logger->info(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void debug(fmt::format_string<Args...> fmt, Args &&...args) {
    m_logger->debug(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> fmt, Args &&...args) {
    m_logger->error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(fmt::format_string<Args...> fmt, Args &&...args) {
    m_logger->warn(fmt, std::forward<Args>(args)...);
  }

  void hexdump(const void *data, size_t size, const char *prefix = "") {
    auto *bytes = static_cast<const uint8_t *>(data);
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
      ascii_line += (bytes[i] >= 32 && bytes[i] <= 126)
                        ? static_cast<char>(bytes[i])
                        : '.';
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

  bool connect(const std::string &ip, uint16_t port, uint32_t timeout_ms) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0)
      return false;

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

    return ::connect(m_socket, (struct sockaddr *)&server_addr,
                     sizeof(server_addr)) == 0;
  }

  bool send(const void *data, size_t size) {
    return ::send(m_socket, data, size, 0) == size;
  }

  // ssize_t recv_full(void* buffer, size_t size) {
  //   return ::recv(m_socket, buffer, size, 0);
  // }

  ssize_t recv_full(void *buffer, size_t expected_size) {
    uint8_t *ptr = static_cast<uint8_t *>(buffer);
    size_t total_bytes_read = 0;

    while (total_bytes_read < expected_size) {
      ssize_t bytes_read = ::recv(m_socket, ptr + total_bytes_read,
                                  expected_size - total_bytes_read, 0);

      if (bytes_read <= 0) {
        if (bytes_read == 0) {
          // m_logger->error("Connection closed unexpectedly during
          // recv_full.");
          std::cerr << "Connection closed unexpectedly during recv_full."
                    << std::endl;
        } else {
          // m_logger->error("Error during recv_full: {}", strerror(errno));
          std::cerr << "Connection closed unexpectedly during recv_full."
                    << std::endl;
        }
        return bytes_read; // Return error or disconnection
      }

      total_bytes_read += bytes_read;
    }

    return total_bytes_read;
  }

private:
  int m_socket;
};

TbtRecoveryClient::TbtRecoveryClient(const std::string &config_file) : m_config_file(config_file) {}

TbtRecoveryClient::~TbtRecoveryClient() = default;

bool TbtRecoveryClient::initialize() { return loadConfig(m_config_file); }

bool TbtRecoveryClient::loadConfig(const std::string &config_file) {
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
        {"CO", Segment::CO}};

    // Load config for each segment
    for (const auto &server : servers) {
      auto segment_str = server.first.as<std::string>();
      auto segment = segment_map.at(segment_str);

      RecoveryConfig cfg{server.second["ip"].as<std::string>(),
                         server.second["port"].as<uint16_t>(),
                         config["recovery"]["timeout_ms"].as<uint32_t>(),
                         config["recovery"]["max_retries"].as<uint32_t>(),
                         config["recovery"]["buffer_size"].as<uint32_t>()};

      m_configs[segment] = cfg;
    }

    return true;
  } catch (const std::exception &e) {
    if (m_logger) {
      m_logger->error("Failed to load config: {}", e.what());
    }
    return false;
  }
}

bool TbtRecoveryClient::requestRecovery(const RecoveryRequest &request) {
  // Get config for requested segment
  auto it = m_configs.find(request.segment);
  if (it == m_configs.end()) {
    m_logger->error("Invalid segment requested");
    return false;
  }
  const auto &config = it->second;

  // Connect to recovery server
  m_connection = std::make_unique<TcpConnection>();
  if (!m_connection->connect(config.server_ip, config.server_port,
                             config.timeout_ms)) {
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
  return processRecovery();
}

bool TbtRecoveryClient::sendRequest(const RecoveryRequestPacket &request) {
  return m_connection->send(&request, sizeof(request));
}

bool TbtRecoveryClient::processRecovery() {
  while (true) {
    // (1) Read the 9-byte StreamHeader
    StreamHeader hdr{};
    ssize_t bytes_read = m_connection->recv_full(&hdr, sizeof(hdr));
    if (bytes_read <= 0) {
      m_logger->error("Connection closed or error while reading TBT header.");
      m_connection.reset();
      break;
    }
    if (bytes_read != static_cast<ssize_t>(sizeof(hdr))) {
      m_logger->error("Incomplete TBT header read: got {} vs expected {}",
                      bytes_read, sizeof(hdr));
      return false;
    }

    // Convert from network to host endianness
    // fixEndianness(hdr); not needed as the data packet is Little Endian.

    // (2) Handling Control Message ('Y')
    /*if (payload[0] == static_cast<uint8_t>(MessageType::Recovery)) {
      auto *recoveryResp = reinterpret_cast<const RecoveryResponse *>(payload.data());
      if (recoveryResp->req_status != 0) { // Assuming '0' means success
        m_logger->error("Recovery request failed with status {}", recoveryResp->req_status);
        return false;
      }
      m_logger->info("TBT Recovery Response Success");
    }*/

    // (3) Read and Process TBT Data
    std::vector<uint8_t> payload(hdr.msg_len);
    size_t total_bytes_read = 0;
    while (total_bytes_read < hdr.msg_len) {
      ssize_t ret = m_connection->recv_full(payload.data() + total_bytes_read,
                                            hdr.msg_len - total_bytes_read);
      if (ret <= 0) {
        m_logger->error("Incomplete TBT packet received: got {} vs expected {}",
                        total_bytes_read, hdr.msg_len);
        return false;
      }
      total_bytes_read += ret;
    }

    m_logger->info("Received TBT packet: seq={}, stream={} size={}", hdr.seq_no,
                   hdr.stream_id, hdr.msg_len);
    m_logger->hexdump(payload.data(), hdr.msg_len, "  ");

    // Process the TBT message

    processTbtMessage(hdr, payload.data(), hdr.msg_len);

    // Fire user callback if set
    if (m_callback) {
      m_callback(hdr.seq_no, payload);
    }

    // Stop if we have received all requested sequences
    if (hdr.seq_no >= m_end_seq) {
      m_logger->info("Reached requested sequence range. Closing connection.");
      return true;
    }
  }

  m_connection.reset();
  return false;
}

void TbtRecoveryClient::processTbtMessage(const StreamHeader &tbtHeader, const uint8_t *payload, size_t length) {
  // We already called fixEndianness(hdr) in processRecovery(),
  // so tbtHeader is in host-endian format.

  MessageType msgType = static_cast<MessageType>(payload[0]);

  switch (msgType) {
  case MessageType::NewOrder:
  case MessageType::ModifyOrder:
  case MessageType::CancelOrder: {
    if (length < sizeof(OrderMessage)) {
      m_logger->error("Payload too small ({} bytes) for OrderMessage", length);
      return;
    }
    auto *orderPtr =
        reinterpret_cast<OrderMessage *>(const_cast<uint8_t *>(payload));
    // fixEndianness(*orderPtr);

    m_logger->info(
        "Order: seq={} stream={} type={} token={} orderid={} price={} qty={}",
        orderPtr->header.seq_no, orderPtr->header.stream_id,
        static_cast<char>(orderPtr->message_type), orderPtr->token,
        orderPtr->order_id, orderPtr->price, orderPtr->quantity);
    break;
  }

  case MessageType::Trade: {
    if (length < sizeof(TradeMessage)) {
      m_logger->error("Payload too small ({} bytes) for TradeMessage", length);
      return;
    }
    auto *tradePtr =
        reinterpret_cast<TradeMessage *>(const_cast<uint8_t *>(payload));
    // fixEndianness(*tradePtr);

    m_logger->info(
        "Trade: seq={} stream={} token={} buyOrd={} sellOrd={} price={} qty={}",
        tradePtr->header.seq_no, tradePtr->header.stream_id, tradePtr->token,
        tradePtr->buy_order_id, tradePtr->sell_order_id, tradePtr->trade_price,
        tradePtr->trade_quantity);
    break;
  }

  case MessageType::PacketLoss: {
    if (length < sizeof(PacketLossPacket)) {
      m_logger->error("Payload too small ({} bytes) for PacketLossPacket",
                      length);
      return;
    }
    auto *lossPtr =
        reinterpret_cast<PacketLossPacket *>(const_cast<uint8_t *>(payload));
    // fixEndianness(*lossPtr);

    m_logger->info("PacketLoss: stream={} from={} to={}",
                   lossPtr->header.stream_id, lossPtr->from_seq,
                   lossPtr->to_seq);
    break;
  }

  case MessageType::HeartBeat: {
    m_logger->debug("Heartbeat received: seq={} stream={}", tbtHeader.seq_no,
                    tbtHeader.stream_id);
    break;
  }
  case MessageType::Recovery: {
    m_logger->warn("Recovery Response received :{}",
                   static_cast<char>(payload[0]));
    break;
  }

  default: {
    m_logger->warn("Unknown message type received: {}",
                   static_cast<char>(payload[0]));
    break;
  }
  }
}

} // namespace recovery
} // namespace acce
