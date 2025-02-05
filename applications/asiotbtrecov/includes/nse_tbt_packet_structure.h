#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace elaeo {
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
struct StreamHeader {
    uint16_t msg_len;
    uint16_t stream_id;
    uint32_t seq_no;
};

struct RecoveryRequestPacket {
    uint8_t     msg_type;    // 'R'
    uint16_t  stream_id;   // 2 bytes
    uint32_t start_seq;   // 4 bytes
    uint32_t end_seq;     // 4 bytes
};

struct RecoveryResponse{
  StreamHeader header;
  uint8_t message_type;
  uint8_t req_status;
};

struct OrderMessage {
    StreamHeader header;
    uint8_t message_type;
    uint64_t timestamp;
    double order_id;
    uint32_t token;
    uint8_t order_type;
    int32_t price;
    uint32_t quantity;
};

struct TradeMessage {
    StreamHeader header;
    uint8_t message_type;
    uint64_t timestamp;
    double buy_order_id;
    double sell_order_id;
    uint32_t token;
    uint32_t trade_price;
    uint32_t trade_quantity;
};

struct PacketLossPacket {
    StreamHeader header;
    uint8_t reserved;
    uint32_t to_seq;
    uint32_t from_seq;
};

#pragma pack(pop)

// Message type identifiers
enum class MessageType : uint8_t {
    NewOrder = 'N',
    ModifyOrder = 'M',
    CancelOrder = 'X',
    Trade = 'T',
    HeartBeat = 'Z',
    SpreadNewOrder = 'G',
    SpreadModifyOrder = 'H',
    SpreadCancelOrder = 'J',
    SpreadTrade = 'K',
    PacketLoss = 'L',
    Recovery = 'Y'
};

// Map segment string to enum
static const std::unordered_map<std::string, elaeo::recovery::Segment>
    segment_map = {{"CM", elaeo::recovery::Segment::CM},
                   {"FO", elaeo::recovery::Segment::FO},
                   {"CD", elaeo::recovery::Segment::CD},
                   {"CO", elaeo::recovery::Segment::CO}};
} // namespace recovery

} // namespace elaeo
