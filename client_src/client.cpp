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
                protocol.send_username(username_param);
                username_sent = true;
                break;
            }
        }
    }

    if (!username_sent) {
        throw std::runtime_error("No username command found in file");
    }

    // SEGUNDO: recibir dinero inicial del servidor (RESPUESTA al username)
    uint8_t response = protocol.recv_command();

    if (response != SEND_INITIAL_MONEY) {
        throw std::runtime_error("Expected initial money from server (0x02), got: 0x" +
                                 std::to_string(response));
    }
    uint32_t money = protocol.recv_initial_money();
    std::cout << "Initial balance: " << (money / 100.0f) << std::endl;  // Sin decimales

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
    protocol.send_get_current_car();  // Envía 0x03 (correcto)

    uint8_t command = protocol.recv_command();

    if (command == SEND_CURRENT_CAR) {
        Car current_car = protocol.recv_current_car();
        print_car_info(current_car, "Current car: ");
    } else if (command == SEND_ERROR_MESSAGE) {
        std::string error = protocol.recv_error_message();
        std::cout << "Error: " << error << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server: 0x" + std::to_string(command));
    }
}

void Client::request_market_info() {
    protocol.send_get_market_info();

    uint8_t command = protocol.recv_command();
    if (command != SEND_MARKET_INFO) {
        throw std::runtime_error("Expected market info from server, got: 0x" +
                                 std::to_string(command));
    }

    std::vector<Car> cars = protocol.recv_market_info();
    print_market_info(cars);
}

void Client::request_buy_car(const std::string& car_name) {
    protocol.send_buy_car(car_name);

    uint8_t command = protocol.recv_command();

    if (command == SEND_CAR_BOUGHT) {
        auto [car, remaining_money] = protocol.recv_car_bought();
        std::cout << "Car bought: " << car.name << ", year: " << car.year
                  << ", price: " << std::fixed << std::setprecision(2) << (car.price / 100.0f)
                  << std::endl;
        std::cout << "Remaining balance: " << (remaining_money / 100.0f)
                  << std::endl;  // Sin decimales
    } else if (command == SEND_ERROR_MESSAGE) {
        std::string error = protocol.recv_error_message();
        std::cout << "Error: " << error << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server: 0x" + std::to_string(command));
    }
}

void Client::print_market_info(const std::vector<Car>& cars) {
    for (const auto& car: cars) {
        std::cout << car.name << ", year: " << car.year << ", price: " << std::fixed
                  << std::setprecision(2) << (float)car.price / 100.0f
                  << std::endl;  // DIVIDIR por 100 solo para mostrar
    }
}

void Client::print_car_info(const Car& car, const std::string& prefix) {
    std::cout << prefix << car.name << ", year: " << car.year << ", price: " << std::fixed
              << std::setprecision(2) << (car.price / 100.0f) << std::endl;  // DIVIDIR por 100
}

void Client::run() {
    // El cliente ya ejecutó todos los comandos en el constructor
}
