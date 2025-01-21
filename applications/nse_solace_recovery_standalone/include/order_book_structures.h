#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct SolaceSnapshotHeader {
    uint16_t transCode;        // First 2 bytes '08 07'
    uint32_t messageSize;        // First 2 bytes '08 07'
    uint32_t numRecs;        // First 2 bytes '08 07'
    uint32_t lastSeqNumber;           // Sequence number
    uint16_t streamId;           // Timestamp
};

struct OrderBookRecord {
    char msgType;
    uint64_t timestamp;            // Unique order ID
    uint64_t orderId;              // Unique order ID
    int token;                     // symbol
    char orderType;                // side (buy/sell)
    uint32_t price;                // price
    uint32_t quantity;             // Quantity
};

#pragma pack(pop)
