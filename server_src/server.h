#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "../common_src/common_protocol.h"
#include "../common_src/common_socket.h"

class Server {
private:
    Socket acceptor_socket;
    std::vector<CarDto> market_cars;
    uint32_t initial_money;

    std::string client_username;
    uint32_t client_money;
    std::optional<CarDto> client_current_car;

    void load_market_data(const std::string& filename);
    void parse_line(const std::string& line);

    // Handlers que trabajan con DTOs
    void handle_user_registration(Protocol& protocol);
    void handle_current_car_request(Protocol& protocol);
    void handle_market_info_request(Protocol& protocol);
    void handle_car_purchase_request(Protocol& protocol);

    const CarDto* find_car_by_name(const std::string& name) const;

public:
    explicit Server(const std::string& port, const std::string& market_file);

    void run();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
};

#endif  // SERVER_H
