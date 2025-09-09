#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>  // <- AGREGAR ESTE INCLUDE

#include "common_socket.h"

// Estructura para representar un auto
struct Car {
    std::string name;
    uint16_t year;
    uint32_t price;  // precio multiplicado por 100 (para evitar float)

    Car(): year(0), price(0) {}
    Car(const std::string& n, uint16_t y, uint32_t p): name(n), year(y), price(p) {}
};

/*
 * Clase Protocol para manejar la serialización y deserialización
 * de mensajes entre cliente y servidor.
 *
 * Esta clase encapsula todos los detalles del protocolo binario.
 */
class Protocol {
private:
    Socket socket;

public:
    explicit Protocol(Socket&& skt);

    // Métodos para enviar mensajes (usado por cliente y servidor)
    void send_username(const std::string& username);
    void send_initial_money(uint32_t money);
    void send_get_current_car();
    void send_current_car(const Car& car);
    void send_get_market_info();
    void send_market_info(const std::vector<Car>& cars);
    void send_buy_car(const std::string& car_name);
    void send_car_bought(const Car& car, uint32_t remaining_money);
    void send_error_message(const std::string& error);

    // Métodos para recibir mensajes (usado por cliente y servidor)
    std::string recv_username();
    uint32_t recv_initial_money();
    void recv_get_current_car();
    Car recv_current_car();
    void recv_get_market_info();
    std::vector<Car> recv_market_info();
    std::string recv_buy_car();
    std::pair<Car, uint32_t> recv_car_bought();  // auto y dinero restante
    std::string recv_error_message();

    // Método genérico para recibir el próximo comando
    uint8_t recv_command();

    // Prohibir copia
    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;

    // Permitir movimiento
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;

private:
    // Métodos auxiliares para conversión de endianness (IMPLEMENTADOS INLINE)
    uint16_t host_to_big_endian_16(uint16_t value) { return htons(value); }

    uint32_t host_to_big_endian_32(uint32_t value) { return htonl(value); }

    uint16_t big_endian_to_host_16(uint16_t value) { return ntohs(value); }

    uint32_t big_endian_to_host_32(uint32_t value) { return ntohl(value); }
};

#endif  // COMMON_PROTOCOL_H
