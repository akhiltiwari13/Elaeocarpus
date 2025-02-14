#include "tbt_packet_structure.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace recovery {

class TbtRecoveryClient {
public:
  struct RecoveryResult {
    bool success;
    uint32_t packets_recovered;
    std::string error_message;
  };

  struct RecoveryRequest {
    uint16_t stream_id;
    uint32_t start_seq;
    uint32_t end_seq;
  };

  explicit TbtRecoveryClient() {
    m_config = {
        "172.28.124.30", // IP address
        10990,           // Port
        5000,            // Timeout in ms
        3,               // Max retries
        65536            // Buffer size
    };
  }

  ~TbtRecoveryClient() {
    if (m_socket >= 0) {
      close(m_socket);
    }
  }

  bool initialize() {
    try {
      m_logger =
          spdlog::daily_logger_mt("simple_logger", "simple_recovery.log");
      m_logger->set_level(spdlog::level::debug);
      m_logger->flush_on(spdlog::level::trace);
      m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
      m_logger->info("TBT Simple Recovery Client initialized");
      return true;
    } catch (const std::exception &e) {
      std::cerr << "Failed to initialize: " << e.what() << std::endl;
      return false;
    }
  }

  void enableDumping(bool enable,
                     const std::string &filename = "tbt_recovery_dump.bin") {
    m_enable_dump = enable;
    m_dump_filename = filename;
  }

  void openDumpFile() {
    if (m_enable_dump && !m_dump_file.is_open()) {
      m_dump_file.open(m_dump_filename, std::ios::binary | std::ios::out);
      if (!m_dump_file.is_open()) {
        m_logger->error("Failed to open dump file: {}", m_dump_filename);
      }
    }
  }

  void dumpToFile(const void *data, size_t size, const char *prefix) {
    if (!m_enable_dump || !m_dump_file.is_open())
      return;

    // Write a separator and prefix
    std::string separator = "\n----------------------------------------\n";
    m_dump_file.write(separator.c_str(), separator.length());
    m_dump_file.write(prefix, strlen(prefix));
    m_dump_file.write("\n", 1);

    // Write the raw data
    m_dump_file.write(static_cast<const char *>(data), size);
    m_dump_file.flush();

    // Also log hexdump if logger is available
    if (m_logger) {
      m_logger->debug("Dumping {} bytes to file with prefix: {}", size, prefix);

      // Create hex dump
      std::stringstream hex_dump;
      const uint8_t *bytes = static_cast<const uint8_t *>(data);

      for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) {
          if (i > 0)
            hex_dump << "\n";
          hex_dump << fmt::format("{:04x}: ", i);
        }
        hex_dump << fmt::format("{:02x} ", bytes[i]);
      }

      m_logger->debug("Hex dump:\n{}", hex_dump.str());
    }
  }

  RecoveryResult performRecovery(const RecoveryRequest &request) {
    RecoveryResult result{false, 0, ""};

    if (!connectToServer()) {
      result.error_message = "Failed to connect to recovery server";
      return result;
    }

    if (!sendRequest(request)) {
      result.error_message = "Failed to send recovery request";
      return result;
    }

    // Wait for and validate recovery response
    StreamHeader hdr{};
    std::vector<uint8_t> payload;

    if (!readNextPacket(hdr, payload)) {
      result.error_message = "Failed to read recovery response";
      return result;
    }

    // Validate recovery response
    if (payload.empty() || payload[0] != 'Y') {
      result.error_message =
          fmt::format("Invalid recovery response type: {}",
                      payload.empty() ? "empty" : std::to_string(payload[0]));
      return result;
    }

    // Verify response status
    if (payload.size() >= 2) {
      uint8_t status = payload[1];
      if (status != 0) {
        result.error_message =
            fmt::format("Recovery request rejected with status: {}", status);
        return result;
      }
    }

    m_logger->info("Recovery request accepted, processing packets...");

    // Process TBT packets
    result.success = true;
    while (true) {
      if (!readNextPacket(hdr, payload)) {
        break;
      }

      if (!processTbtMessage(hdr, payload)) {
        if (result.packets_recovered == 0) {
          result.success = false;
          result.error_message = "Failed to process TBT messages";
          break;
        }
      }

      result.packets_recovered++;

      if (hdr.seq_no >= request.end_seq) {
        m_logger->info("Reached requested sequence range. Closing connection.");
        break;
      }
    }

    return result;
  }

  RecoveryResult performRecoveryDump(const RecoveryRequest &request) {
    RecoveryResult result{false, 0, ""};

    // Open dump file if enabled
    openDumpFile();

    if (!connectToServer()) {
      result.error_message = "Failed to connect to recovery server";
      return result;
    }

    // Dump the recovery request
    if (m_enable_dump) {
      RecoveryRequestPacket req_packet{'R', request.stream_id,
                                       request.start_seq, request.end_seq};
      dumpToFile(&req_packet, sizeof(req_packet), "RECOVERY_REQUEST");
    }

    if (!sendRequest(request)) {
      result.error_message = "Failed to send recovery request";
      return result;
    }

    // Modified packet reading loop
    std::vector<uint8_t> buffer(65536); // Use a large enough buffer
    while (true) {
      // Read data in chunks
      ssize_t bytes_read =
          recv(m_socket, buffer.data(), sizeof(StreamHeader), 0);
      if (bytes_read <= 0)
        break;

      // Dump the header
      if (m_enable_dump) {
        dumpToFile(buffer.data(), bytes_read, "PACKET_HEADER");
      }

      // Parse header
      StreamHeader *hdr = reinterpret_cast<StreamHeader *>(buffer.data());

      // Read payload if there is one
      if (hdr->msg_len > 0) {
        bytes_read = recv(m_socket, buffer.data() + sizeof(StreamHeader),
                          hdr->msg_len, 0);
        if (bytes_read <= 0)
          break;

        // Dump the complete packet (header + payload)
        if (m_enable_dump) {
          dumpToFile(
              buffer.data(), sizeof(StreamHeader) + hdr->msg_len,
              fmt::format("COMPLETE_PACKET_SEQ_{}", hdr->seq_no).c_str());
        }
      }

      result.packets_recovered++;

      // Check if we've reached the end sequence
      if (hdr->seq_no >= request.end_seq) {
        result.success = true;
        break;
      }
    }

    // Close dump file
    if (m_dump_file.is_open()) {
      m_dump_file.close();
    }

    return result;
  }

void parseHexDumpFile(const std::string& filename) {
    auto parse_logger = spdlog::daily_logger_mt("parse_logger", "parsed_data.log", 0, 0);
    parse_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    parse_logger->set_level(spdlog::level::debug);
    parse_logger->flush_on(spdlog::level::debug);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        parse_logger->error("Failed to open hex dump file: {}", filename);
        return;
    }

    parse_logger->info("===============================================");
    parse_logger->info("Starting to parse hex dump file: {}", filename);
    parse_logger->info("===============================================");
    
    std::string line;
    std::vector<uint8_t> current_packet;
    std::string current_section;
    bool in_packet = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Skip lines that don't contain hex data
        if (line.length() < 10 || !std::isxdigit(line[0])) {
            if (line.find("[") != std::string::npos) {
                // This is a section header
                if (!current_packet.empty()) {
                    processPacketData(current_section, current_packet, parse_logger);
                    current_packet.clear();
                }
                current_section = line;
                parse_logger->info("Processing section: {}", line);
            }
            continue;
        }

        // Parse hex line
        // Format: 00000000 50 41 43 4B 45 54 ... |ASCII...|
        std::string hex_part = line.substr(9, 48); // Extract hex portion
        std::istringstream hex_stream(hex_part);
        std::string hex_byte;

        while (hex_stream >> hex_byte) {
            if (hex_byte.length() == 2) {
                try {
                    uint8_t byte = static_cast<uint8_t>(std::stoul(hex_byte, nullptr, 16));
                    current_packet.push_back(byte);
                } catch (const std::exception& e) {
                    parse_logger->warn("Failed to parse hex byte: {}", hex_byte);
                }
            }
        }
    }

    // Process final packet if any
    if (!current_packet.empty()) {
        processPacketData(current_section, current_packet, parse_logger);
    }

    parse_logger->info("===============================================");
    parse_logger->info("Completed parsing hex dump file");
    parse_logger->info("===============================================");
    
    spdlog::drop("parse_logger");
}

void processPacketData(const std::string& section, 
                                        const std::vector<uint8_t>& data,
                                        std::shared_ptr<spdlog::logger> parse_logger) {
    if (data.empty()) {
        parse_logger->warn("Empty packet data for section: {}", section);
        return;
    }

    parse_logger->info("----------------------------------------");
    parse_logger->info("Processing packet data for section: {}", section);

    // Log the raw hex data for verification
    std::stringstream hex_dump;
    for (size_t i = 0; i < data.size(); i++) {
        if (i % 16 == 0) {
            if (i > 0) hex_dump << "\n";
            hex_dump << fmt::format("{:04x}: ", i);
        }
        hex_dump << fmt::format("{:02x} ", data[i]);

        // Add ASCII representation at the end of each line
        if ((i + 1) % 16 == 0 || i == data.size() - 1) {
            // Pad with spaces if needed
            while ((i + 1) % 16 != 0) {
                hex_dump << "   ";
                i++;
            }
            hex_dump << "| ";
            for (size_t j = (i/16)*16; j <= i && j < data.size(); j++) {
                hex_dump << (std::isprint(data[j]) ? static_cast<char>(data[j]) : '.');
            }
        }
    }
    parse_logger->debug("Raw hex dump:\n{}", hex_dump.str());

    // Try to interpret the data based on known packet structures
    if (data.size() >= sizeof(StreamHeader)) {
        const auto* header = reinterpret_cast<const StreamHeader*>(data.data());
        parse_logger->info("Stream Header found:");
        parse_logger->info("  Stream ID: {}", header->stream_id);
        parse_logger->info("  Sequence: {}", header->seq_no);
        parse_logger->info("  Message Length: {}", header->msg_len);

        // If we have more data beyond the header, try to parse the message
        if (data.size() > sizeof(StreamHeader)) {
            uint8_t msg_type = data[sizeof(StreamHeader)];
            parse_logger->info("Message Type: {} (0x{:02x})", 
                             static_cast<char>(msg_type), msg_type);

            switch (msg_type) {
                case 'N':
                case 'M':
                case 'X': {
                    if (data.size() >= sizeof(OrderMessage)) {
                        const auto* order = reinterpret_cast<const OrderMessage*>(data.data());
                        parse_logger->info("Order Message Details:");
                        parse_logger->info("  Type: {}", static_cast<char>(order->message_type));
                        parse_logger->info("  Token: {}", order->token);
                        parse_logger->info("  Order ID: {:.0f}", order->order_id);
                        parse_logger->info("  Price: {}", order->price);
                        parse_logger->info("  Quantity: {}", order->quantity);
                    }
                    break;
                }
                case 'T': {
                    if (data.size() >= sizeof(TradeMessage)) {
                        const auto* trade = reinterpret_cast<const TradeMessage*>(data.data());
                        parse_logger->info("Trade Message Details:");
                        parse_logger->info("  Token: {}", trade->token);
                        parse_logger->info("  Buy Order ID: {:.0f}", trade->buy_order_id);
                        parse_logger->info("  Sell Order ID: {:.0f}", trade->sell_order_id);
                        parse_logger->info("  Price: {}", trade->trade_price);
                        parse_logger->info("  Quantity: {}", trade->trade_quantity);
                    }
                    break;
                }
                case 'Y': {
                    if (data.size() >= sizeof(StreamHeader) + 2) {
                        uint8_t status = data[sizeof(StreamHeader) + 1];
                        parse_logger->info("Recovery Response Status: {}", status);
                    }
                    break;
                }
                default:
                    parse_logger->warn("Unknown message type encountered");
            }
        }
    }
    parse_logger->info("----------------------------------------");
}
private:
  void hexDump(const void *data, size_t size) {
    const unsigned char *p = static_cast<const unsigned char *>(data);
    std::string line;
    for (size_t i = 0; i < size; i++) {
      line += fmt::format("{:02x} ", p[i]);
      if ((i + 1) % 16 == 0 || i == size - 1) {
        m_logger->debug("Hex: {}", line);
        line.clear();
      }
    }
  }

  bool connectToServer() {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
      m_logger->error("Socket creation failed: {}", strerror(errno));
      return false;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(m_socket, F_GETFL, 0);
    if (flags < 0) {
      m_logger->error("Failed to get socket flags: {}", strerror(errno));
      close(m_socket);
      return false;
    }

    if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
      m_logger->error("Failed to set non-blocking mode: {}", strerror(errno));
      close(m_socket);
      return false;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_config.server_port);

    if (inet_pton(AF_INET, m_config.server_ip.c_str(), &server_addr.sin_addr) <=
        0) {
      m_logger->error("Invalid address: {}", strerror(errno));
      close(m_socket);
      return false;
    }

    m_logger->info("Attempting to connect to {}:{}", m_config.server_ip,
                   m_config.server_port);

    int result =
        connect(m_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (result < 0) {
      if (errno != EINPROGRESS) {
        m_logger->error("Connection failed immediately: {}", strerror(errno));
        close(m_socket);
        return false;
      }

      fd_set write_fds;
      FD_ZERO(&write_fds);
      FD_SET(m_socket, &write_fds);

      struct timeval tv{};
      tv.tv_sec = m_config.timeout_ms / 1000;
      tv.tv_usec = (m_config.timeout_ms % 1000) * 1000;

      result = select(m_socket + 1, nullptr, &write_fds, nullptr, &tv);
      if (result <= 0) {
        m_logger->error("Connection timed out after {} ms",
                        m_config.timeout_ms);
        close(m_socket);
        return false;
      }

      int error = 0;
      socklen_t len = sizeof(error);
      if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
          error != 0) {
        m_logger->error("Connection failed during wait: {}",
                        error ? strerror(error) : strerror(errno));
        close(m_socket);
        return false;
      }
    }

    // Return to blocking mode
    if (fcntl(m_socket, F_SETFL, flags & ~O_NONBLOCK) < 0) {
      m_logger->error("Failed to restore blocking mode: {}", strerror(errno));
      close(m_socket);
      return false;
    }

    // Set socket options
    struct timeval tv{};
    tv.tv_sec = m_config.timeout_ms / 1000;
    tv.tv_usec = (m_config.timeout_ms % 1000) * 1000;

    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ||
        setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
      m_logger->error("Failed to set socket timeouts: {}", strerror(errno));
      close(m_socket);
      return false;
    }

    m_logger->info("Successfully connected to {}:{}", m_config.server_ip,
                   m_config.server_port);
    return true;
  }

  bool sendRequest(const RecoveryRequest &request) {
    RecoveryRequestPacket packet;
    packet.msg_type = 'R';
    packet.stream_id = request.stream_id;
    packet.start_seq = request.start_seq;
    packet.end_seq = request.end_seq;

    m_logger->debug("Sending recovery request - Stream: {}, Start: {}, End: {} "
                    "\n hexdump follows...",
                    packet.stream_id, packet.start_seq, packet.end_seq);
    hexDump(&packet, sizeof(packet));
    return send(m_socket, &packet, sizeof(packet), 0) == sizeof(packet);
  }

  bool readExact(void *buffer, size_t size) {
    size_t total_read = 0;
    while (total_read < size) {
      ssize_t bytes_read =
          recv(m_socket, static_cast<uint8_t *>(buffer) + total_read,
               size - total_read, 0);

      if (bytes_read < 0) {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
          // Socket would block, wait a bit and retry
          usleep(1000); // 1ms delay
          continue;
        }
        m_logger->error("Read failed: {}", strerror(errno));
        return false;
      } else if (bytes_read == 0) {
        m_logger->error("Connection closed by peer");
        return false;
      }
      total_read += bytes_read;
    }
    return true;
  }

  bool readNextPacket(StreamHeader &hdr, std::vector<uint8_t> &payload) {
    // Read message length (2 bytes)
    uint16_t msg_len;
    if (!readExact(&msg_len, sizeof(msg_len))) {
      m_logger->error("Failed to read message length");
      return false;
    }

    m_logger->debug("Message length from wire: {}", msg_len);
    hexDump(&msg_len, sizeof(msg_len));

    // Read the complete message
    std::vector<uint8_t> complete_message(msg_len);
    if (!readExact(complete_message.data(), msg_len)) {
      m_logger->error("Failed to read message of length {}", msg_len);
      return false;
    }

    m_logger->debug("Complete message:");
    hexDump(complete_message.data(), complete_message.size());

    // At this point we should have the complete message
    // Find the 'Y' in the message
    auto it = std::find(complete_message.begin(), complete_message.end(), 'Y');
    if (it != complete_message.end()) {
      // Calculate offset to 'Y'
      size_t offset = std::distance(complete_message.begin(), it);
      m_logger->debug("Found 'Y' at offset: {}", offset);

      // Everything from 'Y' onwards is our actual payload
      payload.assign(it, complete_message.end());

      // Header should be just before the 'Y'
      if (offset >= sizeof(StreamHeader)) {
        std::memcpy(&hdr, &complete_message[offset - sizeof(StreamHeader)],
                    sizeof(StreamHeader));
      }
    } else {
      m_logger->error("Failed to find response marker in message");
      return false;
    }

    m_logger->debug("Parsed header - Stream: {}, Seq: {}", hdr.stream_id,
                    hdr.seq_no);

    if (!payload.empty()) {
      m_logger->debug("Parsed payload type: {} ({:#x})",
                      static_cast<char>(payload[0]), payload[0]);
    }

    return true;
  }

  // void processPacketData(const std::string &section,
  //                        const std::vector<uint8_t> &data) {
  //   if (data.empty()) {
  //     m_logger->warn("Empty packet data for section: {}", section);
  //     return;
  //   }
  //
  //   m_logger->debug("Processing section '{}' with {} bytes", section,
  //                   data.size());
  //
  //   if (section.find("PACKET_HEADER") != std::string::npos) {
  //     if (data.size() >= sizeof(StreamHeader)) {
  //       const auto *header =
  //           reinterpret_cast<const StreamHeader *>(data.data());
  //       m_logger->info(
  //           "Stream Header - Stream ID: {}, Sequence: {}, Length: {}",
  //           header->stream_id, header->seq_no, header->msg_len);
  //     }
  //   } else if (section.find("COMPLETE_PACKET_SEQ_") != std::string::npos) {
  //     if (data.size() < sizeof(StreamHeader)) {
  //       m_logger->error("Packet too small for header");
  //       return;
  //     }
  //
  //     const auto *header = reinterpret_cast<const StreamHeader
  //     *>(data.data()); if (data.size() < sizeof(StreamHeader) + 1) {
  //       m_logger->error("Packet too small for message type");
  //       return;
  //     }
  //
  //     uint8_t msg_type = data[sizeof(StreamHeader)];
  //     m_logger->debug("Message Type: {}", static_cast<char>(msg_type));
  //
  //     switch (msg_type) {
  //     case 'N':   // New Order
  //     case 'M':   // Modify Order
  //     case 'X': { // Cancel Order
  //       if (data.size() >= sizeof(OrderMessage)) {
  //         const auto *order =
  //             reinterpret_cast<const OrderMessage *>(data.data());
  //         m_logger->info("Order Message - Type: {}, Token: {}, OrderID: "
  //                        "{:.0f}, Price: {}, Quantity: {}",
  //                        static_cast<char>(order->message_type),
  //                        order->token, order->order_id, order->price,
  //                        order->quantity);
  //       }
  //       break;
  //     }
  //     case 'T': { // Trade
  //       if (data.size() >= sizeof(TradeMessage)) {
  //         const auto *trade =
  //             reinterpret_cast<const TradeMessage *>(data.data());
  //         m_logger->info("Trade Message - Token: {}, Buy Order: {:.0f}, Sell
  //         "
  //                        "Order: {:.0f}, Price: {}, Quantity: {}",
  //                        trade->token, trade->buy_order_id,
  //                        trade->sell_order_id, trade->trade_price,
  //                        trade->trade_quantity);
  //       }
  //       break;
  //     }
  //     case 'Y': { // Recovery Response
  //       if (data.size() >= sizeof(StreamHeader) + 2) {
  //         uint8_t status = data[sizeof(StreamHeader) + 1];
  //         m_logger->info("Recovery Response - Status: {}", status);
  //       }
  //       break;
  //     }
  //     default:
  //       m_logger->warn("Unknown message type: {} (0x{:02x})",
  //                      static_cast<char>(msg_type), msg_type);
  //     }
  //   }
  // }
  bool processTbtMessage(const StreamHeader &header,
                         const std::vector<uint8_t> &payload) {
    if (payload.empty()) {
      m_logger->error("Empty payload received");
      return false;
    }

    uint8_t msgType = payload[0];
    m_logger->debug("Processing message - Type: {} ({:#x}), Header Seq: {}",
                    static_cast<char>(msgType), msgType, header.seq_no);

    // Dump the message content
    hexDump(payload.data(), payload.size());

    switch (msgType) {
    case 'Y': {
      if (payload.size() >= 2) {
        uint8_t status = payload[1];
        m_logger->info("Recovery Response - Status: {} ({:#x})",
                       static_cast<int>(status), status);
        if (status != 0) {
          m_logger->error("Recovery request rejected - Status code: {}",
                          status);
          return (status == 0); // Success only if status is 0
        }
      }
      break;
    }
    case 'N':
    case 'M':
    case 'X': {
      if (payload.size() >= sizeof(OrderMessage) - sizeof(StreamHeader)) {
        auto *order = reinterpret_cast<const OrderMessage *>(payload.data());
        m_logger->info(
            "Order Message - Type: {}, Token: {}, Price: {}, Qty: {}",
            static_cast<char>(order->message_type), order->token, order->price,
            order->quantity);
      }
      break;
    }
    case 'T': {
      if (payload.size() >= sizeof(TradeMessage) - sizeof(StreamHeader)) {
        auto *trade = reinterpret_cast<const TradeMessage *>(payload.data());
        m_logger->info("Trade Message - Token: {}, Price: {}, Qty: {}",
                       trade->token, trade->trade_price, trade->trade_quantity);
      }
      break;
    }
    default:
      m_logger->warn("Unknown message type: {}", static_cast<char>(msgType));
      return false;
    }
    return true;
  }
  // Add this helper function to verify packet structure
  void dumpPacketStructureSizes() {
    m_logger->debug("Structure sizes:");
    m_logger->debug("  StreamHeader: {} bytes", sizeof(StreamHeader));
    m_logger->debug("  RecoveryResponse: {} bytes", sizeof(RecoveryResponse));
    m_logger->debug("  OrderMessage: {} bytes", sizeof(OrderMessage));
    m_logger->debug("  TradeMessage: {} bytes", sizeof(TradeMessage));
  }

  // data members
  int m_socket = -1;
  RecoveryConfig m_config;
  std::shared_ptr<spdlog::logger> m_logger;
  // for response's hex dump
  std::ofstream m_dump_file;
  bool m_enable_dump;
  std::string m_dump_filename;
};

} // namespace recovery

int main() {
  recovery::TbtRecoveryClient client;

  if (!client.initialize()) {
    std::cerr << "Failed to initialize client" << std::endl;
    return 1;
  }

  recovery::TbtRecoveryClient::RecoveryRequest request{
      14,        // stream_id
      217513678, // start_seq
      217514178// end_seq
  };

  // First perform recovery with hex dumping
  std::string dump_filename = "recovery_dump_" +
                              std::to_string(request.stream_id) + "_" +
                              std::to_string(request.start_seq) + "_" +
                              std::to_string(request.end_seq) + ".bin";

  client.enableDumping(true, dump_filename);

  // auto result = client.performRecovery(request);

  // if (!result.success) {
  //   std::cerr << "Recovery failed: " << result.error_message << std::endl;
  //   return 1;
  // }
  //
  // std::cout << "Recovery successful. Recovered " << result.packets_recovered
  //           << " packets." << std::endl;
  //

  std::cout << "Starting recovery with hex dumping..." << std::endl;
  auto dump_result = client.performRecoveryDump(request);

  if (!dump_result.success) {
    std::cerr << "Recovery dump failed: " << dump_result.error_message
              << std::endl;
    return 1;
  }

  std::cout << "Recovery dump successful. Captured "
            << dump_result.packets_recovered << " packets." << std::endl;
  std::cout << "Check the generated dump file for detailed hex output."
            << std::endl;

  std::cout << "parsing hex dump file" << std::endl;
  client.parseHexDumpFile(dump_filename);
  return 0;
}
