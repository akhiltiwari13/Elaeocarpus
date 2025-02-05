// @TODO: split into multiple files.
// networking.hpp
#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <memory>
#include <string>
#include <vector>
#include <span>

namespace modern {
namespace network {

// Forward declarations
class Session;
class SSLSession;

class NetworkClient {
public:
    virtual ~NetworkClient() = default;
    
    // Core interface that all implementations must provide
    virtual void connect(const std::string& host, uint16_t port) = 0;
    virtual void disconnect() = 0;
    virtual std::size_t write(std::span<const uint8_t> data) = 0;
    virtual std::size_t read(std::span<uint8_t> data) = 0;
    
    // Common configuration
    void set_timeout(std::chrono::milliseconds timeout);
    void enable_nodelay(bool enable);
    
protected:
    boost::asio::io_context& get_io_context();
    
private:
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};

// TCP Client Implementation
class TCPClient : public NetworkClient {
public:
    explicit TCPClient(const std::string& local_interface = "");
    
    void connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    std::size_t write(std::span<const uint8_t> data) override;
    std::size_t read(std::span<uint8_t> data) override;
    
    // Additional TCP-specific methods
    void set_keep_alive(bool enable, std::chrono::seconds interval);
    
private:
    boost::asio::ip::tcp::socket socket_;
    std::string local_interface_;
    std::shared_ptr<Session> session_;
};

// SSL Client Implementation
class SSLClient : public NetworkClient {
public:
    explicit SSLClient(const std::string& local_interface = "");
    
    void connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    std::size_t write(std::span<const uint8_t> data) override;
    std::size_t read(std::span<uint8_t> data) override;
    
    // SSL-specific configuration
    void set_verify_mode(boost::asio::ssl::verify_mode mode);
    void set_certificate_file(const std::string& cert_path);
    
private:
    boost::asio::ssl::context ssl_context_;
    std::shared_ptr<SSLSession> session_;
};

// Modern Server Implementation
class Server {
public:
    explicit Server(uint16_t port, const std::string& interface = "");
    
    template<typename Handler>
    void set_connection_handler(Handler&& handler);
    
    void start();
    void stop();
    
private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
};

} // namespace network
} // namespace modern
