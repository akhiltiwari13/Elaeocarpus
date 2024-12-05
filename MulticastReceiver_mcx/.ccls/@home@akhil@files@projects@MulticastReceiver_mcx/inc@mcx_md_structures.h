#pragma once
#include <cstdint>
#include <array>
#include <string_view>


// force 1-byte alignment for all structures.
#pragma pack(push, 1)

enum class TemplateId : uint16_t {
  PACKET_HEADER = 13003,
  HEART_BEAT = 13001,
  AUCTION_CLEARING_PRICE = 13501,
  ORDER_ADD = 13100,
  ORDER_MODIFY = 13101,
  ORDER_MODIFY_SAME_PRIORITY = 13106,
  ORDER_DELETE = 13102,
  ORDER_MASS_DELETE = 13103,
  PARTIAL_ORDER_EXECUTION = 13105,
  FULL_ORDER_EXECUTION = 13104,
  TRADE_EXECUTION_SUMMARY = 13202,
  INSTRUMENT_INFO = 13603,
  PRODUCT_STATE_CHANGE = 13300,
  INSTRUMENT_STATE_CHANGE = 13301,
  SNAPSHOT_PRODUCT_SUMMARY = 13600,
  SNAPSHOT_ORDER = 13602,
  SNAPSHOT_INSTRUMENT_SUMMARY = 13601,
  TOP_OF_BOOK = 13504,
  MASS_INSTRUMENT_STATE_CHANGE = 13302,
  INDEX_INFO = 13604
};

using PriceType = int64_t;
using UTCTimestamp = uint64_t;
using QuantityType = int64_t;

struct MessageHeader {
  uint16_t body_len;
  uint16_t template_id;
  uint32_t msg_seq_num;
};

struct PacketHeader {
  MessageHeader header;
  uint32_t appl_seq_num;
  int32_t market_segment_id;
  uint8_t partition_id;
  uint8_t completion_indicator;
  uint8_t appl_seq_reset_indicator;
  std::array<char, 5> pad5;
  UTCTimestamp transaction_ts;
};

struct HeartBeat {
  MessageHeader header;
  uint32_t last_msg_seq_num_processed;
  std::array<char, 4> pad4;
};

struct OrderAdd {
  MessageHeader header;
  UTCTimestamp exchange_ts;
  int64_t security_id;
  uint64_t reserve2;
  QuantityType quantity;
  uint8_t side;
  uint8_t order_type;
  std::array<char, 6> pad6;
  PriceType price;
};

struct OrderModify {
  MessageHeader header;
  UTCTimestamp exchange_ts;
  UTCTimestamp reserve2;
  PriceType prev_price;
  QuantityType prev_quantity;
  int64_t security_id;
  UTCTimestamp reserve4;
  QuantityType display_qty;
  uint8_t side;
  uint8_t order_type;
  std::array<char, 6> pad6;
  PriceType price;
};

struct OrderModifySamePriority {
  MessageHeader header;
  UTCTimestamp reserve2;
  UTCTimestamp transaction_ts;
  QuantityType prev_qty;
  int64_t security_id;
  UTCTimestamp reserve4;
  QuantityType display_qty;
  uint8_t side;
  uint8_t order_type;
  std::array<char, 6> pad6;
  PriceType price;
};

//@TODO: more msg structures can be added if required to parse.

// Constants and enums for market states
enum class TradingSession : uint8_t {
  Day = 1,
  Morning = 2,
  Evening = 5,
  AfterHours = 6,
  Holiday = 7
};

enum class SessionStatus : uint8_t {
  Halted = 1,
  Open = 2,
  Closed = 3
};

enum class Side : uint8_t {
  Buy = 1,
  Sell = 2
};

inline bool isValidTemplateId(uint16_t id) {
    switch (id) {
        case 13003: // PACKET_HEADER
        case 13001: // HEART_BEAT
        case 13501: // AUCTION_CLEARING_PRICE
        case 13100: // ORDER_ADD
            return true;
        default:
            return false;
    }
}
// Add static assertions to ensure proper structure alignment
static_assert(sizeof(MessageHeader) == 8, "MessageHeader size mismatch");
static_assert(sizeof(PacketHeader) == 32, "PacketHeader size mismatch");
static_assert(sizeof(HeartBeat) == 16, "HeartBeat size mismatch");

#pragma pack(pop)
