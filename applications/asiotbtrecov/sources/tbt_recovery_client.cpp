#include "tbt_recovery_client.h"
#include "nse_tbt_packet_structure.h"
#include <fmt/format.h>
#include <iostream>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <yaml-cpp/yaml.h>

namespace elaeo::recovery {

TbtRecoveryClient::TbtRecoveryClient(asio::io_context &ioc, const std::string &config_file) : m_ioc(ioc), m_socket(ioc), m_timer(ioc), m_config_file(config_file) {}

void TbtRecoveryClient::doConnect(const std::string &host, uint16_t port) {
  m_logger->debug("Attempting connection to {}:{}...", host, port);
  asio::ip::tcp::resolver resolver(m_ioc);
  auto endpoints = resolver.resolve(host, std::to_string(port));

  auto self = shared_from_this(); //capturing it via a shared ptr in the callback so that the client doesn't get destroyed when the callbacks are working. 

  asio::async_connect(
    m_socket, endpoints,
    [this, self](const asio::error_code &ec, const asio::ip::tcp::endpoint &) {
      m_logger->info("Successfully connected to TBT Recovery Server.");
      m_logger->debug("Sending recovery request packet...");
      if (!ec) {
        asio::async_write(m_socket, asio::buffer(&m_current_req_packet, sizeof(RecoveryRequestPacket)),
                          [this](asio::error_code ec, size_t /*bytes*/) {
                          if (!ec){
                          m_logger->debug("Recovery request sent successfully. Starting data read...");
                          startRead();
                          }
                          else {
                          m_logger->error("Write failed: {}", ec.message());
                          m_socket.close();
                          }
                          });
      } else {
        m_logger->error("Connect failed: {}", ec.message());
        m_socket.close();
      }
    });
}

void TbtRecoveryClient::startRead() { 
  m_logger->debug("Waiting for the recovery response packet...");
  readHeader(); }

void TbtRecoveryClient::readHeader() {
  auto self = shared_from_this();
  auto header_buffer = std::make_shared<StreamHeader>();

  m_timer.expires_after(std::chrono::seconds(5)); // 5-second timeout
  m_timer.async_wait([this, self](const asio::error_code &ec) {
    if (!ec) {
      m_logger->error("Timeout waiting for TBT header");
      m_socket.close();
    }
  });

  asio::async_read(m_socket,
                   asio::buffer(header_buffer.get(), sizeof(StreamHeader)),
                   [this, header_buffer](const asio::error_code &ec, std::size_t bytes_transferred) {
                   if (!ec && bytes_transferred == sizeof(StreamHeader)) {
                   m_logger->debug("Received TBT Header: Seq={}, Stream ID={}, Msg Len={}", header_buffer->seq_no, header_buffer->stream_id, header_buffer->msg_len);
                   m_timer.cancel();
                   readPayload(*header_buffer);
                   } else {
                   m_logger->error("Header read failed: {}", ec.message());
                   }
                   });
}

void TbtRecoveryClient::readPayload(const StreamHeader &header) {
  auto self = shared_from_this();
  m_logger->debug("Reading payload for sequence {} (size: {})", header.seq_no, header.msg_len);
  m_read_buffer.resize(header.msg_len);

  asio::async_read(m_socket, asio::buffer(m_read_buffer),
                   [this, self, header](const asio::error_code &ec, std::size_t bytes_transferred) {
                   if (!ec && bytes_transferred == header.msg_len) {
                   handleMessage(header, m_read_buffer);

                   // Continue reading if not reached end sequence
                   if (header.seq_no < m_end_seq) {
                   readHeader();
                   }
                   } else {
                   m_logger->error("Payload read failed: {}", ec.message());
                   }
                   });
}

bool TbtRecoveryClient::initialize() {
  try {
    YAML::Node config = YAML::LoadFile(m_config_file);

    // Setup logger
    auto log_node = config["log"];
    std::string log_file = log_node["file"].as<std::string>();
    std::string log_level = log_node["level"].as<std::string>();

    // Ensure the logger is only created once
    if (!spdlog::get("tbt_logger")) {
      m_logger = spdlog::daily_logger_mt("tbt_logger", log_file);
      m_logger->flush_on(spdlog::level::debug);
      m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    } else {
      m_logger = spdlog::get("tbt_logger");
    }

    m_logger->set_level(spdlog::level::from_str(log_level));
    m_logger->flush_on(spdlog::level::debug);
    m_logger->info("Logger initialized successfully.");
    m_logger->flush();
    
    // Log raw YAML for verification
    std::stringstream ss;
    ss << config;
    m_logger->debug("Full Configuration:\n{}", ss.str());
    

    // Load server configurations
    for (const auto &segment : {"CM", "FO", "CD", "CO"}) {
      RecoveryConfig cfg;
      cfg.server_ip = config["servers"][segment]["ip"].as<std::string>();
      cfg.server_port = config["servers"][segment]["port"].as<uint16_t>();
      cfg.timeout_ms = config["recovery"]["timeout_ms"].as<uint32_t>();
      cfg.max_retries = config["recovery"]["max_retries"].as<uint32_t>();
      cfg.buffer_size = config["recovery"]["buffer_size"].as<uint32_t>();

      // ... populate cfg from YAML ...
      // use of operator[] on a const unordered_map is not allowed because: operator[] can modify the map by inserting elements if the key doesn't exist. With a const map, only const operations are allowed
      // m_configs[elaeo::recovery::segment_map.at(segment)] = cfg;

      // below is the safer method to do the same.
      auto it = segment_map.find(segment);
      if(it != segment_map.end()){
        m_configs[it->second] = cfg;
      }else{
        m_logger->error("couldn't find segment in the map");
      }
    }
    // Log parsed configurations
        for (const auto& [seg, cfg] : m_configs) {
            m_logger->info("Segment: {}, IP: {}, Port: {}, Timeout: {}ms, Max Retries: {}, Buffer Size: {}",
                           static_cast<int>(seg), cfg.server_ip, cfg.server_port,
                           cfg.timeout_ms, cfg.max_retries, cfg.buffer_size);
        }
    return true;
  } catch (const YAML::Exception &e) {
    // Handle errors
    m_logger->error("Exception during initialization: {}", e.what());
    return false;
  }
}
bool TbtRecoveryClient::requestRecovery(const RecoveryRequest &request) {
  auto it = m_configs.find(request.segment);
  if (it == m_configs.end()) {
    m_logger->error("Invalid segment requested");
    return false;
  }

  const auto &config = it->second;
  m_end_seq = request.end_seq;

  m_logger->info("Connecting to recovery server {}:{} for segment {}", config.server_ip, config.server_port, static_cast<int>(request.segment));
  m_logger->debug("Building Recovery request. segment:{}, stream:{}, start_seq:{}, end_seq:{}", static_cast<int>(request.segment), request.stream_id, request.start_seq, request.end_seq);

  // Prepare and send request
  m_current_req_packet = { 'R', request.stream_id, request.start_seq, request.end_seq };
  m_logger->debug("recovery request=> msg_type:{}, stream_id:{}, start_seq:{}, end_seq:{}", m_current_req_packet.msg_type, m_current_req_packet.stream_id, m_current_req_packet.start_seq, m_current_req_packet.end_seq);
  // Connect and send request asynchronously
  doConnect(config.server_ip, config.server_port);

  return true;
}

void TbtRecoveryClient::handleMessage(const StreamHeader& header, const std::vector<uint8_t>& payload) {
    if (payload.empty()) return;

    uint8_t msg_type = payload[0];

    switch (msg_type) {
        case static_cast<uint8_t>(MessageType::NewOrder): {
            const OrderMessage* order = reinterpret_cast<const OrderMessage*>(payload.data());
            m_logger->info("NewOrder: Seq={}, OrderId={}", header.seq_no, order->order_id);

            // Call callback with sequence number and raw packet. check to see if the callback is indeed set.
            if (m_callback) {
                m_callback(header.seq_no, payload);
            }
            break;
        }
        default:
            m_logger->warn("Unknown Message Type: {}", msg_type);
    }
}
} // namespace elaeo::recovery
