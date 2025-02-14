// tbt_packet_structure.h
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace recovery {

enum class Segment { FO };

struct RecoveryConfig {
    std::string server_ip;
    uint16_t server_port;
    uint32_t timeout_ms;
    uint32_t max_retries;
    uint32_t buffer_size;
};

#pragma pack(push, 1)
struct StreamHeader {
    uint16_t msg_len;
    uint16_t stream_id;
    uint32_t seq_no;
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
    uint8_t message_type;
    uint8_t reserved;
    uint32_t to_seq;
    uint32_t from_seq;
};

struct RecoveryRequestPacket {
    uint8_t msg_type;    // 'R'
    uint16_t stream_id;
    uint32_t start_seq;
    uint32_t end_seq;
};

// Recovery response packet - sent by server to acknowledge request
struct RecoveryResponse {
    StreamHeader header;
    uint8_t message_type;    // 'Y' for recovery response
    uint8_t request_status;  // Status of the recovery request
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
} // namespace recovery

