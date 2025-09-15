#include "server.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "../common_src/common_constants.h"

Server::Server(const std::string& port, const std::string& market_file):
        acceptor_socket(port.c_str()), initial_money(0), client_money(0) {
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
        uint32_t money_value;
        iss >> money_value;
        // CORREGIR: Los valores en el archivo están en pesos, convertir a centavos
        initial_money = money_value * 100;
    } else if (command == "car") {
        std::string name;
        uint16_t year;
        uint32_t price;

        iss >> name >> year >> price;
        // CORREGIR: Los precios en el archivo están en pesos, convertir a centavos
        market_cars.emplace_back(name, year, price * 100);
    }
}

void Server::run() {
    Socket client_socket = acceptor_socket.accept();
    Protocol protocol(std::move(client_socket));

    try {
        // PRIMERO: recibir username (primer mensaje del cliente)
        uint8_t first_command = protocol.recv_command();

        if (first_command != SEND_USERNAME) {
            throw std::runtime_error("Expected username as first message (0x01), got: 0x" +
                                     std::to_string(first_command));
        }

        client_username = protocol.recv_username();
        std::cout << "Hello, " << client_username << std::endl;

        // SEGUNDO: enviar dinero inicial (RESPUESTA al username)
        protocol.send_initial_money(client_money);
        // CORREGIR: mostrar en pesos (dividir por 100)
        std::cout << "Initial balance: " << (client_money / 100) << std::endl;

        // LUEGO: procesar otros comandos
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
                    std::cerr << "Unknown command received: 0x" << std::hex << (int)command
                              << std::dec << std::endl;
                    break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Server connection ended: " << e.what() << std::endl;
    }
}

void Server::handle_get_current_car(Protocol& protocol) {
    if (client_current_car.has_value()) {
        protocol.send_current_car(client_current_car.value());
        // CORREGIR: mostrar precio en pesos y formato correcto
        std::cout << "Car " << client_current_car->name << " " << (client_current_car->price / 100)
                  << " " << client_current_car->year << " sent" << std::endl;
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
    auto it = std::find_if(market_cars.begin(), market_cars.end(),
                           [&name](const Car& car) { return car.name == name; });

    if (it != market_cars.end()) {
        return &(*it);  // Devuelve el puntero al elemento encontrado
    }
    return nullptr;  // No encontrado
}
