#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include "nse_tbt_packet_structure.h"
#include <arpa/inet.h>  // ntohs, ntohl

namespace acce {
namespace recovery {

enum class Segment {
  CM,
  FO,
  CD,
  CO
};

// Recovery configuration
struct RecoveryConfig {
  std::string server_ip;
  uint16_t server_port;
  uint32_t timeout_ms;
  uint32_t max_retries;
  uint32_t buffer_size;
};

// Recovery request parameters
struct RecoveryRequest {
  Segment segment;
  uint16_t stream_id;
  uint32_t start_seq;
  uint32_t end_seq;
};

#pragma pack(push, 1)
struct RecoveryRequestPacket {
    uint8_t     msg_type;    // 'R'
    int16_t  stream_id;   // 2 bytes
    uint32_t start_seq;   // 4 bytes
    uint32_t end_seq;     // 4 bytes
};

struct RecoveryResponse{
  StreamHeader header;
  uint8_t message_type;
  uint8_t req_status;
};

#pragma pack(pop)

// endianness helper fucntions:

// Convert 64-bit from network (big-endian) to host
inline uint64_t ntohll(uint64_t be_val)
{
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // On a big-endian system, no swap needed
    return be_val;
#else
    // On little-endian, swap 8 bytes
    uint64_t hi = static_cast<uint64_t>(ntohl(static_cast<uint32_t>(be_val & 0xFFFFFFFFULL))) << 32;
    uint64_t lo = ntohl(static_cast<uint32_t>(be_val >> 32));
    return hi | lo;
#endif
}

// Convert double in big-endian to host double (assuming IEEE 754)
inline double ntohd(double be_double)
{
    union {
        uint64_t i;
        double   d;
    } conv;
    conv.d = be_double;       // copy bits in
    conv.i = ntohll(conv.i);  // swap
    return conv.d;            // read back as host double
}

class TbtRecoveryClient {
public:
  explicit TbtRecoveryClient(const std::string& config_file);
  ~TbtRecoveryClient();

  // Initialize the recovery client
  bool initialize();

  // Request recovery for specified sequence range
  bool requestRecovery(const RecoveryRequest& request);

  // Callback interface for recovery data 
  using RecoveryCallback = std::function<void(uint32_t seq_num, const std::vector<uint8_t>& data)>;
  void setCallback(RecoveryCallback callback) { m_callback = callback; }

private:
  bool loadConfig(const std::string& config_file);
  bool connectToServer(const std::string& ip, uint16_t port);
  bool sendRequest(const RecoveryRequestPacket& request);
  bool processRecovery();
  void processTbtMessage( const StreamHeader& tbtHeader, const uint8_t* payload, size_t length);

  // Configuration
  std::string m_config_file;
  std::unordered_map<Segment, RecoveryConfig> m_configs;
  RecoveryCallback m_callback;

  // TCP socket handling
  class TcpConnection;
  std::unique_ptr<TcpConnection> m_connection;

  // Logging
  class Logger;
  std::unique_ptr<Logger> m_logger;

  //Track the end seq num.
  uint32_t m_end_seq;
};

}
// namespace recovery
} // namespace acce
