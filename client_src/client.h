#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <vector>

#include "../common_src/common_protocol.h"
#include "../common_src/common_socket.h"


class Client {
private:
    Socket socket;
    Protocol protocol;

    void load_and_execute_commands(const std::string& filename);
    void execute_command(const std::string& command, const std::string& parameter);

    void send_username(const std::string& username);
    void request_current_car();
    void request_market_info();
    void request_buy_car(const std::string& car_name);

    void print_car_info(const Car& car, const std::string& prefix = "");
    void print_market_info(const std::vector<Car>& cars);

public:
    Client(const std::string& hostname, const std::string& port, const std::string& commands_file);

    void run();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = default;
    Client& operator=(Client&&) = default;
};

#endif  // CLIENT_H
