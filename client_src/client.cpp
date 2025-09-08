#include "client.h"
#include "../common_src/common_constants.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

Client::Client(const std::string& hostname, const std::string& port, const std::string& commands_file) 
    : socket(hostname.c_str(), port.c_str()), protocol(std::move(socket)) {
    load_and_execute_commands(commands_file);
}

void Client::load_and_execute_commands(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open commands file: " + filename);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::istringstream iss(line);
            std::string command;
            iss >> command;
            
            // Leer el resto de la línea como parámetro
            std::string parameter;
            if (iss >> parameter) {
                execute_command(command, parameter);
            } else {
                execute_command(command, "");
            }
        }
    }
}

void Client::execute_command(const std::string& command, const std::string& parameter) {
    if (command == "username") {
        send_username(parameter);
    } else if (command == "get_current_car") {
        request_current_car();
    } else if (command == "get_market") {
        request_market_info();
    } else if (command == "buy_car") {
        request_buy_car(parameter);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}

void Client::send_username(const std::string& username) {
    protocol.send_username(username);
    
    // Recibir dinero inicial
    uint8_t command = protocol.recv_command();
    if (command != SEND_INITIAL_MONEY) {
        throw std::runtime_error("Expected initial money from server");
    }
    
    uint32_t initial_money = protocol.recv_initial_money();
    std::cout << "Initial balance: " << (initial_money / 100) << std::endl;
}

void Client::request_current_car() {
    protocol.send_get_current_car();
    
    uint8_t command = protocol.recv_command();
    
    if (command == SEND_CURRENT_CAR) {
        Car current_car = protocol.recv_current_car();
        print_car_info(current_car, "Current car: ");
    } else if (command == SEND_ERROR_MESSAGE) {
        std::string error = protocol.recv_error_message();
        std::cout << "Error: " << error << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server");
    }
}

void Client::request_market_info() {
    protocol.send_get_market_info();
    
    uint8_t command = protocol.recv_command();
    if (command != SEND_MARKET_INFO) {
        throw std::runtime_error("Expected market info from server");
    }
    
    std::vector<Car> cars = protocol.recv_market_info();
    print_market_info(cars);
}

void Client::request_buy_car(const std::string& car_name) {
    protocol.send_buy_car(car_name);
    
    uint8_t command = protocol.recv_command();
    
    if (command == SEND_CAR_BOUGHT) {
        auto [car, remaining_money] = protocol.recv_car_bought();
        std::cout << "Car bought: " << car.name << ", year " << car.year 
                  << ", price " << (car.price / 100) << std::endl;
        std::cout << "Remaining balance: " << (remaining_money / 100) << std::endl;
    } else if (command == SEND_ERROR_MESSAGE) {
        std::string error = protocol.recv_error_message();
        std::cout << "Error: " << error << std::endl;
    } else {
        throw std::runtime_error("Unexpected response from server");
    }
}

void Client::print_car_info(const Car& car, const std::string& prefix) {
    std::cout << prefix << car.name << ", year: " << car.year 
              << ", price: " << (car.price / 100) << std::endl;
}

void Client::print_market_info(const std::vector<Car>& cars) {
    for (const auto& car : cars) {
        std::cout << car.name << ", year: " << car.year 
                  << ", price: " << (car.price / 100) << std::endl;
    }
}

void Client::run() {
    // El cliente ya ejecutó todos los comandos en el constructor
    // Este método existe por consistencia con el patrón de diseño
}