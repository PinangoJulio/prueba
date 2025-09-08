#include "client.h"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
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
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }
}