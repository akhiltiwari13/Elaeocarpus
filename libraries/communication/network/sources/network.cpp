// networking.cpp
#include <network.h>

namespace modern {
namespace network {

// TCPClient Implementation
TCPClient::TCPClient(const std::string& local_interface)
    : socket_(get_io_context())
    , local_interface_(local_interface) {
    
    // Set default socket options
    boost::asio::socket_base::keep_alive option(true);
    socket_.set_option(option);
    
    boost::asio::ip::tcp::no_delay nodelay(true);
    socket_.set_option(nodelay);
}

void TCPClient::connect(const std::string& host, uint16_t port) {
    boost::asio::ip::tcp::resolver resolver(get_io_context());
    auto endpoints = resolver.resolve(host, std::to_string(port));
    
    // If local interface is specified, bind before connecting
    if (!local_interface_.empty()) {
        boost::asio::ip::tcp::endpoint local_endpoint(
            boost::asio::ip::address::from_string(local_interface_), 0);
        socket_.open(local_endpoint.protocol());
        socket_.bind(local_endpoint);
    }
    
    // Async connect with timeout
    boost::asio::async_connect(socket_, endpoints,
        [this](const boost::system::error_code& error,
               const boost::asio::ip::tcp::endpoint& endpoint) {
            if (!error) {
                session_ = std::make_shared<Session>(std::move(socket_));
                session_->start();
            }
        });
}

std::size_t TCPClient::write(std::span<const uint8_t> data) {
    boost::system::error_code ec;
    auto bytes_written = boost::asio::write(
        socket_, 
        boost::asio::buffer(data.data(), data.size()),
        ec
    );
    
    if (ec) {
        throw boost::system::system_error(ec);
    }
    
    return bytes_written;
}
