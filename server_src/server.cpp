#include "server.h"
#include "../common_src/common_constants.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

Server::Server(const std::string& port, const std::string& market_file) 
    : acceptor_socket(port.c_str()), initial_money(0), client_money(0) {
    load_market_data(market_file);
    client_money = initial_money;
    std::cout << "Server started" << std::endl;
}

void Server::load_market_data(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open market file: " + filename);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            parse_line(line);
        }
    }
    
    if (initial_money == 0) {
        throw std::runtime_error("No money configuration found in file");
    }
}

void Server::parse_line(const std::string& line) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    if (command == "money") {
        iss >> initial_money;
        // Convertir a centavos (multiplicar por 100)
        initial_money *= 100;
    } else if (command == "car") {
        std::string name;
        uint16_t year;
        uint32_t price;
        
        iss >> name >> year >> price;
        // Convertir precio a centavos (multiplicar por 100)
        price *= 100;
        
        market_cars.emplace_back(name, year, price);
    }
}

void Server::run() {
    Socket client_socket = acceptor_socket.accept();
    Protocol protocol(std::move(client_socket));
    
    // Recibir nombre de usuario
    handle_username(protocol);
    
    // Enviar dinero inicial
    protocol.send_initial_money(client_money);
    std::cout << "Initial balance: " << (client_money / 100) << std::endl;
    
    // Loop principal para manejar comandos
    try {
        while (true) {
            uint8_t command = protocol.recv_command();
            
            switch (command) {
                case GET_CURRENT_CAR:
                    handle_get_current_car(protocol);
                    break;
                case GET_MARKET_INFO:
                    handle_get_market_info(protocol);
                    break;
                case BUY_CAR:
                    handle_buy_car(protocol);
                    break;
                default:
                    std::cerr << "Unknown command received: " << static_cast<int>(command) << std::endl;
                    break;
            }
        }
    } catch (const std::exception& e) {
        // El cliente se desconectó o hubo un error de red
        std::cout << "Client disconnected" << std::endl;
    }
}

void Server::handle_username(Protocol& protocol) {
    uint8_t command = protocol.recv_command();
    if (command != SEND_USERNAME) {
        throw std::runtime_error("Expected username command");
    }
    
    client_username = protocol.recv_username();
    std::cout << "Hello, " << client_username << std::endl;
}

void Server::handle_get_current_car(Protocol& protocol) {
    if (client_current_car.has_value()) {
        protocol.send_current_car(client_current_car.value());
        std::cout << "Car " << client_current_car->name << " " 
                  << (client_current_car->price / 100) << " " 
                  << client_current_car->year << " sent" << std::endl;
    } else {
        protocol.send_error_message("No car bought");
        std::cout << "Error: No car bought" << std::endl;
    }
}

void Server::handle_get_market_info(Protocol& protocol) {
    protocol.send_market_info(market_cars);
    std::cout << market_cars.size() << " cars sent" << std::endl;
}

void Server::handle_buy_car(Protocol& protocol) {
    std::string car_name = protocol.recv_buy_car();
    
    const Car* car = find_car_by_name(car_name);
    if (car == nullptr) {
        protocol.send_error_message("Car not found");
        std::cout << "Error: Car not found" << std::endl;
        return;
    }
    
    if (client_money < car->price) {
        protocol.send_error_message("Insufficient funds");
        std::cout << "Error: Insufficient funds" << std::endl;
        return;
    }
    
    // Comprar el auto
    client_money -= car->price;
    client_current_car = *car;
    
    protocol.send_car_bought(*car, client_money);
    std::cout << "New cars name: " << car->name 
              << " --- remaining balance: " << (client_money / 100) << std::endl;
}

const Car* Server::find_car_by_name(const std::string& name) const {
    for (const auto& car : market_cars) {
        if (car.name == name) {
            return &car;
        }
    }
    return nullptr;
}