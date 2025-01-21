#include "solace_recovery.h"
#include <iostream>

int main() {
    SolaceRecovery recovery;
    
    if (!recovery.initialize()) {
        std::cerr << "Failed to initialize Solace recovery\n";
        return 1;
    }

    std::cout << "Starting order book recovery...\n";
    recovery.recoverAllQueues();
    std::cout << "Recovery complete\n";

    return 0;
}
