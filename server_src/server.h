#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../common_src/common_protocol.h"
#include "../common_src/common_socket.h"


class Server {
private:
    Socket acceptor_socket;
    std::vector<Car> market_cars;
    uint32_t initial_money;

    std::string client_username;
    uint32_t client_money;
    std::optional<Car> client_current_car;

    void load_market_data(const std::string& filename);
    void parse_line(const std::string& line);

    void handle_username(Protocol& protocol);
    void handle_get_current_car(Protocol& protocol);
    void handle_get_market_info(Protocol& protocol);
    void handle_buy_car(Protocol& protocol);

    const Car* find_car_by_name(const std::string& name) const;

public:
    explicit Server(const std::string& port, const std::string& market_file);

    void run();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
};

#endif  // SERVER_H
