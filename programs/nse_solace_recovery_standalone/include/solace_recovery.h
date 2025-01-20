#pragma once
#include <string>
#include <vector>
#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>
#include "order_book_structures.h"

class SolaceRecovery {
public:
    SolaceRecovery();
    ~SolaceRecovery();
    
    bool initialize();
    void recoverAllQueues();
    void shutdown();

private:
    static const char* SOLACE_HOST;
    static const char* SOLACE_VPN;
    static const char* SOLACE_USERNAME;
    static const char* SOLACE_PASSWORD;
    static const std::vector<std::string> QUEUE_NAMES;
    
    void processQueue(const std::string& queueName);
    void processMessage(solClient_opaqueMsg_pt msg);
    void printOrderBook(const uint8_t* data, size_t length);
    void hexDump(const uint8_t* data, size_t length);

    solClient_opaqueContext_pt m_context;
    solClient_opaqueSession_pt m_session;
    
    static solClient_rxMsgCallback_returnCode_t messageReceiveCallback( solClient_opaqueSession_pt opaqueSession_p,
        solClient_opaqueMsg_pt msg_p,
        void* user_p);

    static void eventCallback(solClient_opaqueSession_pt opaqueSession_p,
                            solClient_session_eventCallbackInfo_pt eventInfo_p,
                            void* user_p);

   static void flowEventCallback(
        solClient_opaqueFlow_pt opaqueFlow_p,
        solClient_flow_eventCallbackInfo_pt eventInfo_p,
        void* user_p);
};
