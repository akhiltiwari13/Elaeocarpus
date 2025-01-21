#include "solace_recovery.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>

// Hard-coded Solace configuration based on NSE specs
const char* SolaceRecovery::SOLACE_HOST = "tcp://172.28.124.40:10986";
const char* SolaceRecovery::SOLACE_VPN = "od_fo_prod";
const char* SolaceRecovery::SOLACE_USERNAME = "member_fo_prod";
const char* SolaceRecovery::SOLACE_PASSWORD = "YoEvx#9675";

const std::vector<std::string> SolaceRecovery::QUEUE_NAMES = {
    "lvq.nse.fo.od.1.orderbook",
    // "lvq.nse.fo.od.2.orderbook",
    // "lvq.nse.fo.od.3.orderbook",
    // "lvq.nse.fo.od.4.orderbook",
    // @TODO: Add all queue names here
};

SolaceRecovery::SolaceRecovery() : m_context(nullptr), m_session(nullptr) {}

SolaceRecovery::~SolaceRecovery() {
    shutdown();
}

bool SolaceRecovery::initialize() {
    if (solClient_initialize(SOLCLIENT_LOG_DEFAULT_FILTER, nullptr) != SOLCLIENT_OK) {
        std::cerr << "Failed to initialize Solace client\n";
        return false;
    }

    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;
    if (solClient_context_create(SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                &m_context, &contextFuncInfo, sizeof(contextFuncInfo)) != SOLCLIENT_OK) {
        std::cerr << "Failed to create context\n";
        return false;
    }

    // Setup session properties
    const char* sessionProps[] = {
        SOLCLIENT_SESSION_PROP_HOST, SOLACE_HOST,
        SOLCLIENT_SESSION_PROP_VPN_NAME, SOLACE_VPN,
        SOLCLIENT_SESSION_PROP_USERNAME, SOLACE_USERNAME,
        SOLCLIENT_SESSION_PROP_PASSWORD, SOLACE_PASSWORD,
        nullptr, nullptr
    };

    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;
    sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
    sessionFuncInfo.rxMsgInfo.user_p = this;  // to setup callback
    sessionFuncInfo.eventInfo.callback_p = eventCallback;
    sessionFuncInfo.eventInfo.user_p = this;  
    
    if (solClient_session_create((char**)sessionProps, m_context, &m_session,
                                &sessionFuncInfo, sizeof(sessionFuncInfo)) != SOLCLIENT_OK) {
        std::cerr << "Failed to create session\n";
        return false;
    }

    // if (solClient_session_connect(m_session) != SOLCLIENT_OK) {
    //     std::cerr << "Failed to connect session\n";
    //     return false;
    // }

  if (solClient_session_connect(m_session) != SOLCLIENT_OK) {
    solClient_errorInfo_pt errorInfo = solClient_getLastErrorInfo();
    std::cerr << "Failed to connect session: " 
              << solClient_subCodeToString(errorInfo->subCode)
              << " (" << errorInfo->errorStr << ")" << std::endl;
    return false;
}
    return true;
}

void SolaceRecovery::recoverAllQueues() {
    for (const auto& queueName : QUEUE_NAMES) {
        processQueue(queueName);
    }
}

void SolaceRecovery::flowEventCallback(
    solClient_opaqueFlow_pt /*opaqueFlow_p*/,
    solClient_flow_eventCallbackInfo_pt eventInfo_p,
    void* /*user_p*/) 
{
    std::cout << "Flow event: " 
              << solClient_flow_eventToString(eventInfo_p->flowEvent)
              << std::endl;
}

void SolaceRecovery::processQueue(const std::string& queueName) {
    std::cout << "\nProcessing queue: " << queueName << std::endl;

    solClient_opaqueFlow_pt flow_p = nullptr;
    const char* flowProps[] = {
        SOLCLIENT_FLOW_PROP_BIND_BLOCKING, SOLCLIENT_PROP_ENABLE_VAL,
        SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID, SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE,
        SOLCLIENT_FLOW_PROP_BIND_NAME, queueName.c_str(),
        SOLCLIENT_FLOW_PROP_BROWSER, SOLCLIENT_PROP_ENABLE_VAL,
        SOLCLIENT_FLOW_PROP_WINDOWSIZE, "1",
        nullptr, nullptr
    };

    solClient_flow_createFuncInfo_t flowFuncInfo = SOLCLIENT_FLOW_CREATEFUNC_INITIALIZER;
    
    // Use correct callback functions for flow
    flowFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
    flowFuncInfo.rxMsgInfo.user_p = this;
    flowFuncInfo.eventInfo.callback_p = flowEventCallback;  // Use flow-specific callback
    flowFuncInfo.eventInfo.user_p = this;

    if (solClient_session_createFlow((char**)flowProps, m_session, &flow_p,
                                    &flowFuncInfo, sizeof(flowFuncInfo)) != SOLCLIENT_OK) {
        solClient_errorInfo_pt errorInfo = solClient_getLastErrorInfo();
        std::cerr << "Failed to create flow for queue: " << queueName 
                  << " - " << solClient_subCodeToString(errorInfo->subCode)
                  << " (" << errorInfo->errorStr << ")" << std::endl;
        return;
    }

    // Wait for message processing
    usleep(1000000);  // Wait for 1 second

    // Cleanup flow
    solClient_flow_destroy(&flow_p);
}

void SolaceRecovery::processMessage(solClient_opaqueMsg_pt msg) {
    void* data;
    solClient_uint32_t length;
    
    if (solClient_msg_getBinaryAttachmentPtr(msg, &data, &length) != SOLCLIENT_OK) {
        std::cerr << "Failed to get message data\n";
        return;
    }

    std::cout << "\nReceived message with length: " << length << " bytes\n\n";
    
    // Print raw hex dump for verification
    std::cout << "Raw message dump:\n";
    hexDump(static_cast<const uint8_t*>(data), length);
    
    // Parse message header
    if (length < sizeof(SolaceSnapshotHeader)) {
        std::cerr << "Message too small for header\n";
        return;
    }

    const auto* header = static_cast<const SolaceSnapshotHeader*>(data);
    std::cout << "\nMessage Header:\n"
              << "Length: " << header->messageSize << "\n"
              << "Sequence: " << header->lastSeqNumber << "\n"
              << "StreamId: " << header->streamId << "\n"
              << "NumRecs:" << std::hex << header->numRecs << std::dec 
              << "\n\n";

    // Process order book records
    const uint8_t* recordPtr = static_cast<const uint8_t*>(data) + sizeof(SolaceSnapshotHeader);
    size_t remainingLength = length - sizeof(SolaceSnapshotHeader);
    int recordCount = 0;

    while (remainingLength >= sizeof(OrderBookRecord)) {
        const auto* record = reinterpret_cast<const OrderBookRecord*>(recordPtr);
        
        std::cout << "Order Record #" << ++recordCount << ":\n"
                  << "  msg_type: " << record->msg_type << "\n";
                  << "  timestamp: " << record->timestamp << "\n";
                  << "  Order ID: " << record->orderId << "\n"
                  << "  token: " << record->token << "\n"
                  << "  order type: " << record->orderType << "\n"
                  << "  Price: " << (record->price / 100.0) << "\n"
                  << "  Quantity: " << record->quantity << "\n";
                  // << "  Flags: 0x" << std::hex << record-> << std::dec << "\n\n";

        recordPtr += sizeof(OrderBookRecord);
        remainingLength -= sizeof(OrderBookRecord);
    }

    if (remainingLength > 0) {
        std::cout << "Note: " << remainingLength 
                  << " bytes of message data remaining\n";
    }
}

void SolaceRecovery::hexDump(const uint8_t* data, size_t length) {
    char ascii[17] = {0};
    size_t i, j;

    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            if (i != 0) printf("  %s\n", ascii);
            printf("  %04zx:", i);
        }

        printf(" %02x", data[i]);
        ascii[i % 16] = (data[i] >= ' ' && data[i] <= '~') ? data[i] : '.';
        ascii[(i % 16) + 1] = '\0';

        if ((i + 1) % 8 == 0 || i + 1 == length) {
            printf(" ");
            if ((i + 1) % 16 == 0) {
                printf(" ");
            }
        }
    }

    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    printf("  %s\n", ascii);
    printf("\n");
}

void SolaceRecovery::printOrderBook(const uint8_t* data, size_t length) {
    if (length < sizeof(SolaceSnapshotHeader)) {
        std::cerr << "Message too small for header\n";
        return;
    }

    const SolaceSnapshotHeader* header = reinterpret_cast<const SolaceSnapshotHeader*>(data);
    std::cout << "Order Book Header:\n"
              << "  Stream ID: " << header->streamId << "\n"
              << "  Last Sequence num: " << header->lastSeqNumber << "\n"
              << "  Records: " << header->numRecs << "\n\n";

    const uint8_t* orderData = data + sizeof(SolaceSnapshotHeader);
    size_t remainingLength = length - sizeof(SolaceSnapshotHeader);

    for (uint32_t i = 0; i < header->numRecs && remainingLength >= sizeof(OrderBookRecord); i++) {
        const OrderBookRecord* entry = reinterpret_cast<const OrderBookRecord*>(orderData);
        std::cout << "Order " << i + 1 << ":\n"
                  << "  Order ID: " << entry->orderId << "\n"
                  << "  Side: " << entry->orderType << "\n"
                  << "  Price: " << entry->price / 100.0 << "\n"
                  << "  Quantity: " << entry->quantity << "\n\n";

        orderData += sizeof(OrderBookRecord);
        remainingLength -= sizeof(OrderBookRecord);
    }
}

solClient_rxMsgCallback_returnCode_t SolaceRecovery::messageReceiveCallback( solClient_opaqueSession_pt /*opaqueSession_p*/, solClient_opaqueMsg_pt msg_p, void* user_p) 
{
    auto* recovery = static_cast<SolaceRecovery*>(user_p);
    recovery->processMessage(msg_p);
    return SOLCLIENT_CALLBACK_OK;  // Add this return value
}

void SolaceRecovery::eventCallback(solClient_opaqueSession_pt /*opaqueSession_p*/,
                                 solClient_session_eventCallbackInfo_pt eventInfo_p,
                                 void* /*user_p*/) {
    std::cout << "Session event: " 
              << solClient_session_eventToString(eventInfo_p->sessionEvent)
              << std::endl;
}

void SolaceRecovery::shutdown() {
    if (m_session) {
        solClient_session_disconnect(m_session);
        solClient_session_destroy(&m_session);
    }
    if (m_context) {
        solClient_context_destroy(&m_context);
    }
    solClient_cleanup();
}
