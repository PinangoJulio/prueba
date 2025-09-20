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
        initial_money = money_value;
    } else if (command == "car") {
        std::string name;
        uint16_t year;
        uint32_t price;

        iss >> name >> year >> price;
        // RAII aplicado: constructor apropiado
        market_cars.emplace_back(name, year, price * 100);  // precio en centavos
    }
}

void Server::run() {
    Socket client_socket = acceptor_socket.accept();
    Protocol protocol(std::move(client_socket));

    try {
        // PRIMERO: manejar registro de usuario
        handle_user_registration(protocol);

        // LUEGO: procesar comandos del negocio
        while (true) {
            uint8_t command = protocol.receive_command();

            switch (command) {
                case GET_CURRENT_CAR:
                    handle_current_car_request(protocol);
                    break;
                case GET_MARKET_INFO:
                    handle_market_info_request(protocol);
                    break;
                case BUY_CAR:
                    handle_car_purchase_request(protocol);
                    break;
                default:
                    std::cerr << "Unknown command received: 0x" << std::hex << (int)command
                              << std::dec << std::endl;
                    break;
            }
        }
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        if (error_msg.find("Client disconnected") != std::string::npos) {
            std::cerr << "Server connection ended: Client disconnected" << std::endl;
        } else {
            std::cerr << "Server connection ended: " << e.what() << std::endl;
        }
    }
}

// ==== HANDLERS QUE TRABAJAN CON DTOs ====

void Server::handle_user_registration(Protocol& protocol) {
    uint8_t first_command = protocol.receive_command();

    if (first_command != SEND_USERNAME) {
        throw std::runtime_error("Expected username as first message");
    }

    // NUEVO: Recibir como DTO
    UserDto user = protocol.receive_user_registration();
    client_username = user.username;
    std::cout << "Hello, " << client_username << std::endl;

    // NUEVO: Enviar dinero como DTO
    MoneyDto initial_balance(initial_money);
    protocol.send_initial_balance(initial_balance);
    std::cout << "Initial balance: " << initial_money << std::endl;

    client_money = initial_money;
}

void Server::handle_current_car_request(Protocol& protocol) {
    if (client_current_car.has_value()) {
        // NUEVO: Enviar auto como DTO
        protocol.send_current_car_info(client_current_car.value());

        // Mostrar precio en pesos (dividir por 100)
        std::cout << "Car " << client_current_car->name << " " << (client_current_car->price / 100)
                  << " " << client_current_car->year << " sent" << std::endl;
    } else {
        // NUEVO: Enviar error como DTO
        ErrorDto error("No car bought");
        protocol.send_error_notification(error);
        std::cout << "Error: No car bought" << std::endl;
    }
}

void Server::handle_market_info_request(Protocol& protocol) {
    // NUEVO: Enviar market como DTO
    MarketDto market(market_cars);
    protocol.send_market_catalog(market);
    std::cout << market_cars.size() << " cars sent" << std::endl;
}

void Server::handle_car_purchase_request(Protocol& protocol) {
    // NUEVO: Recibir nombre del auto directamente (no como DTO porque es un parámetro simple)
    std::string car_name = protocol.receive_car_purchase_request();

    const CarDto* car = find_car_by_name(car_name);
    if (car == nullptr) {
        ErrorDto error("Car not found");
        protocol.send_error_notification(error);
        std::cout << "Error: Car not found" << std::endl;
        return;
    }

    // Verificar fondos (convertir precio a pesos para comparar)
    if (client_money < (car->price / 100)) {
        ErrorDto error("Insufficient funds");
        protocol.send_error_notification(error);
        std::cout << "Error: Insufficient funds" << std::endl;
        return;
    }

    // Comprar el auto - trabajar en pesos
    client_money -= (car->price / 100);
    client_current_car = *car;

    // NUEVO: Enviar confirmación como DTO
    CarPurchaseDto purchase(*car, client_money);
    protocol.send_purchase_confirmation(purchase);

    std::cout << "New cars name: " << car->name << " --- remaining balance: " << client_money
              << std::endl;
}

const CarDto* Server::find_car_by_name(const std::string& name) const {
    auto it = std::find_if(market_cars.begin(), market_cars.end(),
                           [&name](const CarDto& car) { return car.name == name; });

    if (it != market_cars.end()) {
        return &(*it);
    }
    return nullptr;
}
