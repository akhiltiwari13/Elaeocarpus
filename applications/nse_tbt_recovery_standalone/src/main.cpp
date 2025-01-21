#include "tbt_recovery_client.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>

void printUsage(const char* program) {
  std::cout << "Usage: " << program << " <config_file> <segment> <stream_id> <start_seq> <end_seq>\n";
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

    // Map segment string to enum
    static const std::unordered_map<std::string, acce::recovery::Segment> segment_map = {
      {"CM", acce::recovery::Segment::CM},
      {"FO", acce::recovery::Segment::FO},
      {"CD", acce::recovery::Segment::CD},
      {"CO", acce::recovery::Segment::CO}
    };

    auto segment_it = segment_map.find(segment_str);
    if (segment_it == segment_map.end()) {
      std::cerr << "Invalid segment: " << segment_str << std::endl;
      return 1;
    }

    // Initialize recovery client
    acce::recovery::TbtRecoveryClient client(config_file);
    if (!client.initialize()) {
      std::cerr << "Failed to initialize recovery client" << std::endl;
      return 1;
    }

    // Set callback to print recovered packets
    client.setCallback([](uint32_t seq_num, const std::vector<uint8_t>& data) {
      std::cout << "Recovered packet: seq=" << seq_num 
                << " size=" << data.size() << std::endl;
    });

    // Request recovery
    acce::recovery::RecoveryRequest request{
      segment_it->second,
      stream_id,
      start_seq,
      end_seq
    };

    if (!client.requestRecovery(request)) {
      std::cerr << "Recovery request failed" << std::endl;
      return 1;
    }

    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
