#pragma once
#include <cstdint>

namespace acce {
namespace recovery {

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
} // namespace acce
