// tbt_recovery_client.hpp
#pragma once
#include "nse_tbt_packet_structure.h"
#include <asio.hpp>
#include <asio/steady_timer.hpp>
#include <memory>
#include <vector>
#include <string>
#include <spdlog/logger.h>

namespace elaeo::recovery {

// recovery client singleton.
class TbtRecoveryClient : public std::enable_shared_from_this<TbtRecoveryClient> {
public:
  //single static instance.
  static std::shared_ptr<TbtRecoveryClient> create(asio::io_context &ioc, const std::string &config_file) 
  {
    return std::shared_ptr<TbtRecoveryClient>( new TbtRecoveryClient(ioc, config_file));
  }

  using RecoveryCallback = std::function<void(uint32_t seq_num, const std::vector<uint8_t> &data)>;

  bool initialize();
  bool requestRecovery(const RecoveryRequest &request);
  void setCallback(RecoveryCallback callback) { m_callback = callback; }

private:
  // private constructor.
  explicit TbtRecoveryClient(asio::io_context &ioc, const std::string &config_file);

  void doConnect(const std::string &host, uint16_t port);
  void startRead();
  void readHeader();
  void readPayload(const StreamHeader &header);
  void handleMessage(const StreamHeader &header,
                     const std::vector<uint8_t> &payload);

  asio::io_context &m_ioc;
  asio::ip::tcp::socket m_socket;
  asio::steady_timer m_timer;

  std::string m_config_file;
  std::unordered_map<Segment, RecoveryConfig> m_configs;
  RecoveryCallback m_callback;
  uint32_t m_end_seq;
  std::shared_ptr<spdlog::logger> m_logger;
  std::vector<uint8_t> m_read_buffer;
  std::vector<uint8_t> m_header_buffer;
  std::vector<uint8_t> m_payload_buffer;
  elaeo::recovery::RecoveryRequestPacket m_current_req_packet;
};

} // namespace elaeo::recovery
