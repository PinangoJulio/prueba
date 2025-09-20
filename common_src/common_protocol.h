#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <arpa/inet.h>

#include "common_socket.h"

// DTOs (Data Transfer Objects) - Objetos que entiende el negocio
struct UserDto {
    std::string username;

    UserDto() = default;
    explicit UserDto(const std::string& name): username(name) {}
};

struct CarDto {
    std::string name;
    uint16_t year;
    uint32_t price;  // En centavos

    // Aplicando RAII - constructor que inicializa apropiadamente
    CarDto(): name(""), year(0), price(0) {}
    CarDto(const std::string& n, uint16_t y, uint32_t p): name(n), year(y), price(p) {}
};

struct MoneyDto {
    uint32_t amount;

    MoneyDto(): amount(0) {}
    explicit MoneyDto(uint32_t amt): amount(amt) {}
};

struct MarketDto {
    std::vector<CarDto> cars;

    MarketDto() = default;
    explicit MarketDto(const std::vector<CarDto>& car_list): cars(car_list) {}
};

struct CarPurchaseDto {
    CarDto car;
    uint32_t remaining_money;

    CarPurchaseDto(): remaining_money(0) {}
    CarPurchaseDto(const CarDto& c, uint32_t money): car(c), remaining_money(money) {}
};

struct ErrorDto {
    std::string message;

    ErrorDto() = default;
    explicit ErrorDto(const std::string& msg): message(msg) {}
};

// Buffer para serialización - UN ÚNICO PAQUETE
class MessageBuffer {
private:
    std::vector<uint8_t> buffer;

public:
    MessageBuffer() { buffer.reserve(1024); }  // RAII - reserva inicial

    void clear() { buffer.clear(); }

    void append_byte(uint8_t value);
    void append_uint16(uint16_t value);
    void append_uint32(uint32_t value);
    void append_string(const std::string& str);
    void append_car(const CarDto& car);

    const uint8_t* data() const { return buffer.data(); }
    size_t size() const { return buffer.size(); }
};

class Protocol {
private:
    Socket socket;
    MessageBuffer send_buffer;

    // Métodos privados de serialización
    void serialize_user(const UserDto& user);
    void serialize_money(const MoneyDto& money);
    void serialize_car(const CarDto& car);
    void serialize_market(const MarketDto& market);
    void serialize_car_purchase(const CarPurchaseDto& purchase);
    void serialize_error(const ErrorDto& error);

    // Métodos privados de deserialización
    UserDto deserialize_user();
    MoneyDto deserialize_money();
    CarDto deserialize_car();
    MarketDto deserialize_market();
    CarPurchaseDto deserialize_car_purchase();
    ErrorDto deserialize_error();

    // Endianness helpers
    uint16_t host_to_big_endian_16(uint16_t value) { return htons(value); }
    uint32_t host_to_big_endian_32(uint32_t value) { return htonl(value); }
    uint16_t big_endian_to_host_16(uint16_t value) { return ntohs(value); }
    uint32_t big_endian_to_host_32(uint32_t value) { return ntohl(value); }

public:
    explicit Protocol(Socket&& skt);

    // Interfaz descriptiva que trabaja con DTOs
    void send_user_registration(const UserDto& user);
    void send_initial_balance(const MoneyDto& money);
    void send_current_car_info(const CarDto& car);
    void send_market_catalog(const MarketDto& market);
    void send_purchase_confirmation(const CarPurchaseDto& purchase);
    void send_error_notification(const ErrorDto& error);

    // Requests (sin parámetros adicionales)
    void send_current_car_request();
    void send_market_info_request();
    void send_car_purchase_request(const std::string& car_name);

    // Métodos de recepción
    uint8_t receive_command();
    UserDto receive_user_registration();
    MoneyDto receive_initial_balance();
    CarDto receive_current_car_info();
    MarketDto receive_market_catalog();
    CarPurchaseDto receive_purchase_confirmation();
    ErrorDto receive_error_notification();
    std::string receive_car_purchase_request();

    // Una sola llamada a sendall por mensaje
    void flush_message(uint8_t command_code);

    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;
};

#endif  // COMMON_PROTOCOL_H
