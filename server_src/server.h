#ifndef SERVER_H
#define SERVER_H

#include "../common_src/common_socket.h"
#include "../common_src/common_protocol.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <optional>

/*
 * Clase Server que maneja la lógica del servidor del juego Need for Speed.
 * Se encarga de cargar los datos del mercado, manejar las conexiones de clientes
 * y procesar los comandos recibidos.
 */
class Server {
private:
    Socket acceptor_socket;
    std::vector<Car> market_cars;
    uint32_t initial_money;
    
    // Estado por cliente (en esta PoC solo hay un cliente)
    std::string client_username;
    uint32_t client_money;
    std::optional<Car> client_current_car;
    
    // Métodos privados para carga de datos
    void load_market_data(const std::string& filename);
    void parse_line(const std::string& line);
    
    // Métodos privados para manejo de comandos
    void handle_username(Protocol& protocol);
    void handle_get_current_car(Protocol& protocol);
    void handle_get_market_info(Protocol& protocol);
    void handle_buy_car(Protocol& protocol);
    
    // Método auxiliar para buscar auto por nombre
    const Car* find_car_by_name(const std::string& name) const;

public:
    explicit Server(const std::string& port, const std::string& market_file);
    
    void run();
    
    // Prohibir copia
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    
    // Permitir movimiento
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
};

#endif // SERVER_H