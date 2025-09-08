#ifndef CLIENT_H
#define CLIENT_H

#include "../common_src/common_socket.h"
#include "../common_src/common_protocol.h"
#include <string>
#include <vector>
#include <iostream>

/*
 * Clase Client que maneja la lógica del cliente del juego Need for Speed.
 * Se encarga de conectarse al servidor, cargar y ejecutar los comandos
 * desde un archivo, y mostrar los resultados por pantalla.
 */
class Client {
private:
    Socket socket;
    Protocol protocol;
    
    // Métodos privados para carga y ejecución de comandos
    void load_and_execute_commands(const std::string& filename);
    void execute_command(const std::string& command, const std::string& parameter);
    
    // Métodos privados para manejo de comandos específicos
    void send_username(const std::string& username);
    void request_current_car();
    void request_market_info();
    void request_buy_car(const std::string& car_name);
    
    // Métodos privados para mostrar información
    void print_car_info(const Car& car, const std::string& prefix = "");
    void print_market_info(const std::vector<Car>& cars);

public:
    Client(const std::string& hostname, const std::string& port, const std::string& commands_file);
    
    void run();
    
    // Prohibir copia
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    
    // Permitir movimiento
    Client(Client&&) = default;
    Client& operator=(Client&&) = default;
};

#endif // CLIENT_H