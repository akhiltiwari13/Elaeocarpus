#include "mcast_channel.h"
#include "mcx_debug.h"
#include "mcx_md_structures.h"
#include <filesystem>
#include <iostream>
#include <signal.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

using namespace std::chrono;
// Global flag for signal handling
volatile sig_atomic_t running = 1;

void signalHandler(int signum) { running = 0; }

class MCXReceiver {
private:
  struct Config {
    std::string multicast_group;
    uint16_t port;
    std::string interface_ip;
    bool blocking;
    int stream_id;
    std::string log_file;
  };

  Config config_;
  std::unique_ptr<MulticastChannel> mc_;
  std::shared_ptr<spdlog::logger> logger_;
  uint32_t last_seq_num_ = 0;
  UTCTimestamp last_exchange_time_ = 0;
  bool sequence_initialized_{false};

  void setupLogger() {
    try {
      std::string log_dir = "logs";
      if (!std::filesystem::exists(log_dir)) {
        std::filesystem::create_directories(log_dir);
      }

      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( log_dir + "/market_data.log", 5 * 1024 * 1024, 3);

      std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
      logger_ = std::make_shared<spdlog::logger>("mcx_receiver", sinks.begin(), sinks.end());
      logger_->set_level(spdlog::level::info);
      spdlog::set_default_logger(logger_);

      logger_->info("Logger initialized successfully");
    } catch (const std::exception &e) {
      throw std::runtime_error(std::string("Failed to setup logger: ") + e.what());
    }
  }

  void loadConfig(const std::string &config_path) {
    try {
      auto yaml = YAML::LoadFile(config_path);

      // Load configuration before setting up logger
      config_.multicast_group = yaml["connection"]["multicast_group"].as<std::string>();
      config_.port = yaml["connection"]["port"].as<uint16_t>();
      config_.interface_ip = yaml["connection"]["interface_ip"].as<std::string>();
      config_.blocking = yaml["connection"]["blocking"].as<bool>();
      config_.stream_id = yaml["connection"]["stream_id"].as<int>();
      config_.log_file = yaml["logging"]["log_file"].as<std::string>();

      // Setup logger after loading config
      setupLogger();

      logger_->info("Loaded config - Group: {}, Port: {}, IP: {}", config_.multicast_group, config_.port, config_.interface_ip);
    } catch (const YAML::Exception &e) {
      throw std::runtime_error("Failed to parse config: " + std::string(e.what()));
    }
  }

  std::string getTemplateIdName(uint16_t template_id) {
    switch (template_id) {
    case static_cast<uint16_t>(TemplateId::PACKET_HEADER): return "PACKET_HEADER";
    case static_cast<uint16_t>(TemplateId::HEART_BEAT): return "HEART_BEAT";
    case static_cast<uint16_t>(TemplateId::AUCTION_CLEARING_PRICE): return "AUCTION_CLEARING_PRICE";
    case static_cast<uint16_t>(TemplateId::ORDER_ADD): return "ORDER_ADD";
    case static_cast<uint16_t>(TemplateId::ORDER_MODIFY): return "ORDER_MODIFY";
    case static_cast<uint16_t>(TemplateId::ORDER_MODIFY_SAME_PRIORITY): return "ORDER_MODIFY_SAME_PRIORITY";
    case static_cast<uint16_t>(TemplateId::ORDER_DELETE): return "ORDER_DELETE";
    case static_cast<uint16_t>(TemplateId::ORDER_MASS_DELETE): return "ORDER_MASS_DELETE";
    case static_cast<uint16_t>(TemplateId::PARTIAL_ORDER_EXECUTION): return "PARTIAL_ORDER_EXECUTION";
    case static_cast<uint16_t>(TemplateId::FULL_ORDER_EXECUTION): return "FULL_ORDER_EXECUTION";
    case static_cast<uint16_t>(TemplateId::TRADE_EXECUTION_SUMMARY): return "TRADE_EXECUTION_SUMMARY";
    default: return "UNKNOWN_" + std::to_string(template_id);
    }
  }

void processPacketHeader(const char* data, size_t length) {
    if (length < sizeof(PacketHeader)) return;

    auto* packet = reinterpret_cast<const PacketHeader*>(data);
    logger_->info("Packet Header Details:");
    logger_->info("  Market Segment: {}", packet->market_segment_id);
    logger_->info("  Message Sequence: {}", packet->header.msg_seq_num);
    logger_->info("  Application Sequence: {}", packet->appl_seq_num);
    logger_->info("  Partition ID: {}", packet->partition_id);
    logger_->info("  Transaction Time: {}", packet->transaction_ts);

    // Initialize sequence number on first packet
    if (!sequence_initialized_) {
        last_seq_num_ = packet->header.msg_seq_num;
        sequence_initialized_ = true;
        logger_->info("Initializing sequence tracking with number: {}", last_seq_num_);
        return;
    }

    checkSequenceGap(packet->header.msg_seq_num);
}

void processOrderAdd(const char* data, size_t length) {
    if (length < sizeof(OrderAdd)) return;

    auto* order = reinterpret_cast<const OrderAdd*>(data);
    logger_->info("Order Add Details:");
    logger_->info("  Security ID: {}", order->security_id);
    logger_->info("  Price: {}", order->price);
    logger_->info("  Quantity: {}", order->quantity);
    logger_->info("  Side: {}", (order->side == 1 ? "Buy" : "Sell"));
    logger_->info("  Exchange Time: {}", order->exchange_ts);
}


  void processHeartbeat(const HeartBeat* hb) {
    // Get current system time with nanosecond precision
    auto system_time = high_resolution_clock::now();
    auto system_ns = duration_cast<nanoseconds>(system_time.time_since_epoch()).count();

    // Log the complete heartbeat message structure
    logger_->info("Heartbeat Message Details:");
    logger_->info("  Message Length: {}", hb->header.body_len);
    logger_->info("  Template ID: {}", hb->header.template_id);
    logger_->info("  Sequence Number: {}", hb->header.msg_seq_num);
    logger_->info("  Last Processed Sequence: {}", hb->last_msg_seq_num_processed);

    // Log timestamps with nanosecond precision
    logger_->info("Timestamp Comparison:");
    logger_->info("  System Time (ns): {}", system_ns);
    logger_->info("  Time Difference: {} ns", 
                  system_ns - static_cast<int64_t>(last_exchange_time_));

    // Update the last exchange time
    last_exchange_time_ = system_ns;
  }

void checkSequenceGap(uint32_t received_seq) {
    if (!sequence_initialized_) {
        last_seq_num_ = received_seq;
        sequence_initialized_ = true;
        logger_->info("Initializing sequence number to: {}", received_seq);
        return;
    }

    uint32_t expected_seq = last_seq_num_ + 1;
    
    // Handle sequence number wraparound
    if (received_seq < last_seq_num_ && last_seq_num_ > 0xFFFFFF00) {
        logger_->info("Sequence wrapped around - Last: {}, New: {}", 
                     last_seq_num_, received_seq);
    }
    else if (received_seq != expected_seq) {
        logger_->warn("Sequence gap detected:");
        logger_->warn("  Last Sequence: {}", last_seq_num_);
        logger_->warn("  Expected: {}", expected_seq);
        logger_->warn("  Received: {}", received_seq);
        logger_->warn("  Gap Size: {}", 
                     received_seq > expected_seq ? 
                     received_seq - expected_seq : 
                     0xFFFFFFFF - expected_seq + received_seq + 1);
    }

    last_seq_num_ = received_seq;
}

void processMessage(const char* data, size_t length) {
    if (length < sizeof(MessageHeader)) {
        logger_->warn("Message too small: received {} bytes, minimum required {}", 
                     length, sizeof(MessageHeader));
        return;
    }

    auto* msg_header = reinterpret_cast<const MessageHeader*>(data);
    auto template_id = static_cast<TemplateId>(msg_header->template_id);

    // First, handle standalone messages
    if (template_id == TemplateId::HEART_BEAT) {
        if (length >= sizeof(HeartBeat)) {
            logger_->info("Raw Heartbeat Data:");
            MCXDebugger::hexDump(data, sizeof(HeartBeat), logger_);
            processHeartbeat(reinterpret_cast<const HeartBeat*>(data));
            return;
        }
    }

    // Then handle packet messages
    const char* msg_data = data;
    size_t remaining_length = length;

    while (remaining_length >= sizeof(MessageHeader)) {
        auto* current_header = reinterpret_cast<const MessageHeader*>(msg_data);
        
        if (current_header->body_len > remaining_length) {
            break;
        }

        switch (static_cast<TemplateId>(current_header->template_id)) {
            case TemplateId::PACKET_HEADER:
                processPacketHeader(msg_data, remaining_length);
                break;
            case TemplateId::ORDER_ADD:
                processOrderAdd(msg_data, remaining_length);
                break;
            default:
                logger_->debug("Unhandled message type: {}", 
                             getTemplateIdName(current_header->template_id));
                break;
        }

        msg_data += current_header->body_len;
        remaining_length -= current_header->body_len;
    }
}
public:
  MCXReceiver(const std::string &config_path) {
    loadConfig(config_path); // Load config first, which will setup logger
    mc_ = std::make_unique<MulticastChannel>(
        config_.multicast_group, config_.port, config_.interface_ip,
        config_.blocking, config_.stream_id);
  }
  void start() {
    mc_->start();
    logger_->info("MCX receiver started");

    std::vector<char> buffer(2048);
    while (running) {
      auto bytes_read = mc_->readData(buffer.data(), buffer.size());
      if (bytes_read > 0) {
        processMessage(buffer.data(), bytes_read);
      }
    }

    logger_->info("Shutting down MCX receiver");
    mc_->stop();
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <config_file.yaml>" << std::endl;
    return 1;
  }

  // Setup signal handling
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  try {
    MCXReceiver receiver(argv[1]);
    receiver.start();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
