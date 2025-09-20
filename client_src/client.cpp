#include "client.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "../common_src/common_constants.h"

Client::Client(const std::string& hostname, const std::string& port,
               const std::string& commands_file):
        socket(hostname.c_str(), port.c_str()), protocol(std::move(socket)) {
    load_and_execute_commands(commands_file);
}

void Client::load_and_execute_commands(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open commands file: " + filename);
    }

    // PRIMERO: encontrar y enviar el comando username
    std::string line;
    bool username_sent = false;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::istringstream iss(line);
            std::string command;
            iss >> command;

            if (command == "username") {
                std::string username_param;
                iss >> username_param;

                // NUEVO: Trabajar con DTOs
                UserDto user(username_param);
                protocol.send_user_registration(user);
                username_sent = true;
                break;
            }
        }
    }

    if (!username_sent) {
        throw std::runtime_error("No username command found in file");
    }

    // SEGUNDO: recibir dinero inicial del servidor
    uint8_t response = protocol.receive_command();
    if (response != SEND_INITIAL_MONEY) {
        throw std::runtime_error("Expected initial money from server");
    }

    // NUEVO: Recibir como DTO
    MoneyDto initial_money = protocol.receive_initial_balance();
    std::cout << "Initial balance: " << initial_money.amount << std::endl;

    // TERCERO: reiniciar y ejecutar todos los comandos
    file.clear();
    file.seekg(0);

    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::istringstream iss(line);
            std::string command;
            iss >> command;

            if (command == "username")
                continue;  // Ya procesado

            std::string parameter;
            std::getline(iss, parameter);
            parameter.erase(0, parameter.find_first_not_of(" \t"));

            execute_command(command, parameter);
        }
    }
}

void Client::execute_command(const std::string& command, const std::string& parameter) {
    if (command == "get_current_car") {
        request_current_car();
    } else if (command == "get_market") {
        request_market_info();
    } else if (command == "buy_car") {
        request_buy_car(parameter);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}

void Client::request_current_car() {
    protocol.send_current_car_request();

    uint8_t command = protocol.receive_command();

    if (command == SEND_CURRENT_CAR) {
        // NUEVO: Recibir como DTO
        CarDto current_car = protocol.receive_current_car_info();
        print_car_info(current_car, "Current car: ");
    } else if (command == SEND_ERROR_MESSAGE) {
        // NUEVO: Recibir error como DTO
        ErrorDto error = protocol.receive_error_notification();
        std::cout << "Error: " << error.message << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server");
    }
}

void Client::request_market_info() {
    protocol.send_market_info_request();

    uint8_t command = protocol.receive_command();
    if (command != SEND_MARKET_INFO) {
        throw std::runtime_error("Expected market info from server");
    }

    // NUEVO: Recibir como DTO
    MarketDto market = protocol.receive_market_catalog();
    print_market_info(market.cars);
}

void Client::request_buy_car(const std::string& car_name) {
    protocol.send_car_purchase_request(car_name);

    uint8_t command = protocol.receive_command();

    if (command == SEND_CAR_BOUGHT) {
        // NUEVO: Recibir como DTO
        CarPurchaseDto purchase = protocol.receive_purchase_confirmation();

        std::cout << "Car bought: " << purchase.car.name << ", year: " << purchase.car.year
                  << ", price: " << std::fixed << std::setprecision(2)
                  << (purchase.car.price / 100.0f) << std::endl;
        std::cout << "Remaining balance: " << purchase.remaining_money << std::endl;

    } else if (command == SEND_ERROR_MESSAGE) {
        // NUEVO: Recibir error como DTO
        ErrorDto error = protocol.receive_error_notification();
        std::cout << "Error: " << error.message << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server");
    }
}

void Client::print_market_info(const std::vector<CarDto>& cars) {
    for (const auto& car: cars) {
        std::cout << car.name << ", year: " << car.year << ", price: " << std::fixed
                  << std::setprecision(2) << (car.price / 100.0f) << std::endl;
    }
}

void Client::print_car_info(const CarDto& car, const std::string& prefix) {
    std::cout << prefix << car.name << ", year: " << car.year << ", price: " << std::fixed
              << std::setprecision(2) << (car.price / 100.0f) << std::endl;
}

void Client::run() {}
