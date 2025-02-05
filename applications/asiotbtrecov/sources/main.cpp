#include <asio.hpp>
#include "tbt_recovery_client.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>

void printUsage(const char* program) {
  std::cout << "Usage help: " << program << " <config_file> <segment> <stream_id> <start_seq> <end_seq>\n";
  std::cout << "Segments: CM, FO, CD, CO\n";
}

int main(int argc, char* argv[]) {
  if (argc != 6) {
    printUsage(argv[0]);
    return 1;
  }
  

  try {
    // Parse command line arguments
    std::string config_file = argv[1];
    std::string segment_str = argv[2];
    uint16_t stream_id = std::stoi(argv[3]);
    uint32_t start_seq = std::stoul(argv[4]);
    uint32_t end_seq = std::stoul(argv[5]);



    auto segment_it = elaeo::recovery::segment_map.find(segment_str.c_str());
    // check if the segment is valid.
    if (segment_it == elaeo::recovery::segment_map.end()) {
      std::cerr << "Invalid segment: " << segment_str << std::endl;
      return 1;
    }

    // create asio context & client singleton
    asio::io_context ioc;
    auto client = elaeo::recovery::TbtRecoveryClient::create(ioc, config_file); 

    std::cout << "Initializing recovery client..." << std::endl;
    // initialize client singleton
    if (!client->initialize()) {
      std::cerr << "Failed to initialize recovery client" << std::endl;
      return 1;
    }
    std::cout << "Recovery client initialized..." << std::endl;
    // Set Callback to print recovered packets. @TODO find where is this callback called?
    client->setCallback([](uint32_t seq_num, const std::vector<uint8_t>& data) {
      std::cout << "Recovered packet: seq=" << seq_num << " size=" << data.size() << std::endl;
    });

    // Request Recovery
    elaeo::recovery::RecoveryRequest request{
      segment_it->second,
      stream_id,
      start_seq,
      end_seq
    };

    if (!client->requestRecovery(request)) {
      std::cerr << "Recovery request failed" << std::endl;
      return 1;
    }

    // Stop io_context once recovery is complete
    // asio::post(ioc, [&ioc]() { ioc.stop(); });

    // async run for the recovery
    ioc.run();
    // spdlog::shutdown();

    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
