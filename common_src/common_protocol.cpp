#include "common_protocol.h"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "common_constants.h"

Protocol::Protocol(Socket&& skt): socket(std::move(skt)) {}

// ========== MÉTODOS PARA ENVIAR ==========

void Protocol::send_username(const std::string& username) {
    uint8_t command = SEND_USERNAME;
    uint16_t length = host_to_big_endian_16(username.length());

    socket.sendall(&command, sizeof(command));
    socket.sendall(&length, sizeof(length));
    socket.sendall(username.c_str(), username.length());
}

void Protocol::send_initial_money(uint32_t money) {
    uint8_t command = SEND_INITIAL_MONEY;
    uint32_t money_be = host_to_big_endian_32(money);

    socket.sendall(&command, sizeof(command));
    socket.sendall(&money_be, sizeof(money_be));
}

void Protocol::send_get_current_car() {
    uint8_t command = GET_CURRENT_CAR;
    socket.sendall(&command, sizeof(command));
}

void Protocol::send_current_car(const Car& car) {
    uint8_t command = SEND_CURRENT_CAR;
    uint16_t name_length = host_to_big_endian_16(car.name.length());
    uint16_t year_be = host_to_big_endian_16(car.year);
    uint32_t price_be = host_to_big_endian_32(car.price);

    socket.sendall(&command, sizeof(command));
    socket.sendall(&name_length, sizeof(name_length));
    socket.sendall(car.name.c_str(), car.name.length());
    socket.sendall(&year_be, sizeof(year_be));
    socket.sendall(&price_be, sizeof(price_be));
}

void Protocol::send_get_market_info() {
    uint8_t command = GET_MARKET_INFO;
    socket.sendall(&command, sizeof(command));
}

void Protocol::send_market_info(const std::vector<Car>& cars) {
    uint8_t command = SEND_MARKET_INFO;
    uint16_t num_cars = host_to_big_endian_16(cars.size());

    socket.sendall(&command, sizeof(command));
    socket.sendall(&num_cars, sizeof(num_cars));

    for (const auto& car: cars) {
        uint16_t name_length = host_to_big_endian_16(car.name.length());
        uint16_t year_be = host_to_big_endian_16(car.year);
        uint32_t price_be = host_to_big_endian_32(car.price);

        socket.sendall(&name_length, sizeof(name_length));
        socket.sendall(car.name.c_str(), car.name.length());
        socket.sendall(&year_be, sizeof(year_be));
        socket.sendall(&price_be, sizeof(price_be));
    }
}

void Protocol::send_buy_car(const std::string& car_name) {
    uint8_t command = BUY_CAR;
    uint16_t length = host_to_big_endian_16(car_name.length());

    socket.sendall(&command, sizeof(command));
    socket.sendall(&length, sizeof(length));
    socket.sendall(car_name.c_str(), car_name.length());
}

void Protocol::send_car_bought(const Car& car, uint32_t remaining_money) {
    uint8_t command = SEND_CAR_BOUGHT;
    uint16_t name_length = host_to_big_endian_16(car.name.length());
    uint16_t year_be = host_to_big_endian_16(car.year);
    uint32_t price_be = host_to_big_endian_32(car.price);
    uint32_t money_be = host_to_big_endian_32(remaining_money);

    socket.sendall(&command, sizeof(command));
    socket.sendall(&name_length, sizeof(name_length));
    socket.sendall(car.name.c_str(), car.name.length());
    socket.sendall(&year_be, sizeof(year_be));
    socket.sendall(&price_be, sizeof(price_be));
    socket.sendall(&money_be, sizeof(money_be));
}

void Protocol::send_error_message(const std::string& error) {
    uint8_t command = SEND_ERROR_MESSAGE;
    uint16_t length = host_to_big_endian_16(error.length());

    socket.sendall(&command, sizeof(command));
    socket.sendall(&length, sizeof(length));
    socket.sendall(error.c_str(), error.length());
}

// ========== MÉTODOS PARA RECIBIR ==========

uint8_t Protocol::recv_command() {
    uint8_t command = 0;
    int ret = socket.recvall(&command, sizeof(command));
    if (ret == 0) {
        throw std::runtime_error("Client disconnected");
    }

    return command;
}

std::string Protocol::recv_username() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string username(length, '\0');
    socket.recvall(&username[0], length);
    return username;
}

uint32_t Protocol::recv_initial_money() {
    uint32_t money;
    socket.recvall(&money, sizeof(money));
    money = big_endian_to_host_32(money);
    return money;
}

void Protocol::recv_get_current_car() {
    // No hay datos adicionales que recibir
}

Car Protocol::recv_current_car() {
    uint16_t name_length;
    socket.recvall(&name_length, sizeof(name_length));
    name_length = big_endian_to_host_16(name_length);

    std::string name(name_length, '\0');
    socket.recvall(&name[0], name_length);

    uint16_t year;
    socket.recvall(&year, sizeof(year));
    year = big_endian_to_host_16(year);

    uint32_t price;
    socket.recvall(&price, sizeof(price));
    price = big_endian_to_host_32(price);

    return Car(name, year, price);
}

void Protocol::recv_get_market_info() {
    // No hay datos adicionales que recibir
}

std::vector<Car> Protocol::recv_market_info() {
    uint16_t num_cars;
    socket.recvall(&num_cars, sizeof(num_cars));
    num_cars = big_endian_to_host_16(num_cars);

    std::vector<Car> cars;
    cars.reserve(num_cars);

    for (int i = 0; i < num_cars; i++) {
        uint16_t name_length;
        socket.recvall(&name_length, sizeof(name_length));
        name_length = big_endian_to_host_16(name_length);

        std::string name(name_length, '\0');
        socket.recvall(&name[0], name_length);

        uint16_t year;
        socket.recvall(&year, sizeof(year));
        year = big_endian_to_host_16(year);

        uint32_t price;
        socket.recvall(&price, sizeof(price));
        price = big_endian_to_host_32(price);

        cars.emplace_back(name, year, price);
    }

    return cars;
}

std::string Protocol::recv_buy_car() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string car_name(length, '\0');
    socket.recvall(&car_name[0], length);
    return car_name;
}

std::pair<Car, uint32_t> Protocol::recv_car_bought() {
    // Primero leemos la info del auto
    uint16_t name_length;
    socket.recvall(&name_length, sizeof(name_length));
    name_length = big_endian_to_host_16(name_length);

    std::string name(name_length, '\0');
    socket.recvall(&name[0], name_length);

    uint16_t year;
    socket.recvall(&year, sizeof(year));
    year = big_endian_to_host_16(year);

    uint32_t price;
    socket.recvall(&price, sizeof(price));
    price = big_endian_to_host_32(price);

    Car car(name, year, price);

    uint32_t remaining_money;
    socket.recvall(&remaining_money, sizeof(remaining_money));
    remaining_money = big_endian_to_host_32(remaining_money);

    return std::make_pair(car, remaining_money);
}

std::string Protocol::recv_error_message() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string error(length, '\0');
    socket.recvall(&error[0], length);
    return error;
}
