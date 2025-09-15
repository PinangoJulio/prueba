#include <exception>
#include <iostream>

#include "client.h"

int main(int argc, const char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port> <commands-file>" << std::endl;
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];
    std::string commands_file = argv[3];

    try {
        Client client(hostname, port, commands_file);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
