// Example client code
int main() {
    try {
        modern::network::TCPClient client;
        
        // Configure the client
        client.set_timeout(std::chrono::seconds(5));
        client.enable_nodelay(true);
        
        // Connect
        client.connect("127.0.0.1", 8080);
        
        // Send data
        std::vector<uint8_t> data = {1, 2, 3, 4, 5};
        client.write(data);
        
        // Receive response
        std::vector<uint8_t> response(1024);
        auto bytes_read = client.read(response);
        
    } catch (const boost::system::system_error& e) {
        std::cerr << "Network error: " << e.what() << std::endl;
    }
}

// Example server code
int main() {
    modern::network::Server server(8080);
    
    server.set_connection_handler([](auto&& session) {
        session->async_read([](auto&& data) {
            // Process data
            // Send response
        });
    });
    
    server.start();
}
