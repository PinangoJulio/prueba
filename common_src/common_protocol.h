#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>

#include "common_socket.h"

struct Car {
    std::string name;
    uint16_t year;
    uint32_t price;

    Car(): year(0), price(0) {}
    Car(const std::string& n, uint16_t y, uint32_t p): name(n), year(y), price(p) {}
};

class Protocol {
private:
    Socket socket;

public:
    explicit Protocol(Socket&& skt);

    void send_username(const std::string& username);
    void send_initial_money(uint32_t money);
    void send_get_current_car();
    void send_current_car(const Car& car);
    void send_get_market_info();
    void send_market_info(const std::vector<Car>& cars);
    void send_buy_car(const std::string& car_name);
    void send_car_bought(const Car& car, uint32_t remaining_money);
    void send_error_message(const std::string& error);
    void send_current_car_request();

    std::string recv_username();
    uint32_t recv_initial_money();
    void recv_get_current_car();
    Car recv_current_car();
    void recv_get_market_info();
    std::vector<Car> recv_market_info();
    std::string recv_buy_car();
    std::pair<Car, uint32_t> recv_car_bought();
    std::string recv_error_message();

    uint8_t recv_command();

    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;

    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;

private:
    uint16_t host_to_big_endian_16(uint16_t value) { return htons(value); }

    uint32_t host_to_big_endian_32(uint32_t value) { return htonl(value); }

    uint16_t big_endian_to_host_16(uint16_t value) { return ntohs(value); }

    uint32_t big_endian_to_host_32(uint32_t value) { return ntohl(value); }
};

#endif  // COMMON_PROTOCOL_H
