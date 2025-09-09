#include <exception>
#include <iostream>

#include "server.h"

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <market-file>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    std::string market_file = argv[2];

    try {
        Server server(port, market_file);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;  // ← CAMBIAR de 0 a 1
    }
    return 0;
}
