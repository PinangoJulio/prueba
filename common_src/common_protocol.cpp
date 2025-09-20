#include "common_protocol.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include "common_constants.h"

// MessageBuffer implementation
void MessageBuffer::append_byte(uint8_t value) { buffer.push_back(value); }

void MessageBuffer::append_uint16(uint16_t value) {
    uint16_t network_value = htons(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&network_value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(uint16_t));
}

void MessageBuffer::append_uint32(uint32_t value) {
    uint32_t network_value = htonl(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&network_value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(uint32_t));
}

void MessageBuffer::append_string(const std::string& str) {
    append_uint16(str.length());
    buffer.insert(buffer.end(), str.begin(), str.end());
}

void MessageBuffer::append_car(const CarDto& car) {
    append_string(car.name);
    append_uint16(car.year);
    append_uint32(car.price);
}

// Protocol implementation
Protocol::Protocol(Socket&& skt): socket(std::move(skt)) {}

// ==== ENVÍO - Trabajando con DTOs y UN solo sendall ====

void Protocol::send_user_registration(const UserDto& user) {
    send_buffer.clear();
    serialize_user(user);
    flush_message(SEND_USERNAME);
}

void Protocol::send_initial_balance(const MoneyDto& money) {
    send_buffer.clear();
    serialize_money(money);
    flush_message(SEND_INITIAL_MONEY);
}

void Protocol::send_current_car_info(const CarDto& car) {
    send_buffer.clear();
    serialize_car(car);
    flush_message(SEND_CURRENT_CAR);
}

void Protocol::send_market_catalog(const MarketDto& market) {
    send_buffer.clear();
    serialize_market(market);
    flush_message(SEND_MARKET_INFO);
}

void Protocol::send_purchase_confirmation(const CarPurchaseDto& purchase) {
    send_buffer.clear();
    serialize_car_purchase(purchase);
    flush_message(SEND_CAR_BOUGHT);
}

void Protocol::send_error_notification(const ErrorDto& error) {
    send_buffer.clear();
    serialize_error(error);
    flush_message(SEND_ERROR_MESSAGE);
}

void Protocol::send_current_car_request() {
    send_buffer.clear();  // Asegurar buffer limpio aunque no haya datos
    flush_message(GET_CURRENT_CAR);
}

void Protocol::send_market_info_request() {
    send_buffer.clear();  // Asegurar buffer limpio aunque no haya datos
    flush_message(GET_MARKET_INFO);
}

void Protocol::send_car_purchase_request(const std::string& car_name) {
    send_buffer.clear();
    send_buffer.append_string(car_name);
    flush_message(BUY_CAR);
}

// ==== FLUSH - Una sola llamada a sendall ====
void Protocol::flush_message(uint8_t command_code) {
    // Crear buffer completo: comando + datos
    std::vector<uint8_t> complete_message;
    complete_message.reserve(1 + send_buffer.size());

    // Agregar comando
    complete_message.push_back(command_code);

    // Agregar datos serializados
    if (send_buffer.size() > 0) {
        const uint8_t* data = send_buffer.data();
        complete_message.insert(complete_message.end(), data, data + send_buffer.size());
    }

    // ¡UNA SOLA LLAMADA A SENDALL!
    socket.sendall(complete_message.data(), complete_message.size());
}

// ==== SERIALIZACIÓN ====
void Protocol::serialize_user(const UserDto& user) { send_buffer.append_string(user.username); }

void Protocol::serialize_money(const MoneyDto& money) { send_buffer.append_uint32(money.amount); }

void Protocol::serialize_car(const CarDto& car) { send_buffer.append_car(car); }

void Protocol::serialize_market(const MarketDto& market) {
    send_buffer.append_uint16(market.cars.size());
    for (const auto& car: market.cars) {
        send_buffer.append_car(car);
    }
}

void Protocol::serialize_car_purchase(const CarPurchaseDto& purchase) {
    send_buffer.append_car(purchase.car);
    send_buffer.append_uint32(purchase.remaining_money);
}

void Protocol::serialize_error(const ErrorDto& error) { send_buffer.append_string(error.message); }

// ==== RECEPCIÓN ====
uint8_t Protocol::receive_command() {
    uint8_t command = 0;
    int ret = socket.recvall(&command, sizeof(command));
    if (ret == 0) {
        throw std::runtime_error("Client disconnected");
    }
    return command;
}

UserDto Protocol::receive_user_registration() { return deserialize_user(); }

MoneyDto Protocol::receive_initial_balance() { return deserialize_money(); }

CarDto Protocol::receive_current_car_info() { return deserialize_car(); }

MarketDto Protocol::receive_market_catalog() { return deserialize_market(); }

CarPurchaseDto Protocol::receive_purchase_confirmation() { return deserialize_car_purchase(); }

ErrorDto Protocol::receive_error_notification() { return deserialize_error(); }

std::string Protocol::receive_car_purchase_request() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string car_name(length, '\0');
    socket.recvall(&car_name[0], length);
    return car_name;
}

// ==== DESERIALIZACIÓN ====
UserDto Protocol::deserialize_user() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string username(length, '\0');
    socket.recvall(&username[0], length);
    return UserDto(username);
}

MoneyDto Protocol::deserialize_money() {
    uint32_t amount;
    socket.recvall(&amount, sizeof(amount));
    amount = big_endian_to_host_32(amount);
    return MoneyDto(amount);
}

CarDto Protocol::deserialize_car() {
    uint16_t name_length;
    socket.recvall(&name_length, sizeof(name_length));
    name_length = big_endian_to_host_16(name_length);

    std::string name(name_length, '\0');
    socket.recvall(&name[0], name_length);

    uint16_t year;
    socket.recvall(&year, sizeof(year));
    year = big_endian_to_host_16(year);

    uint32_t price;
    socket.recvall(&price, sizeof(price));
    price = big_endian_to_host_32(price);

    return CarDto(name, year, price);
}

MarketDto Protocol::deserialize_market() {
    uint16_t num_cars;
    socket.recvall(&num_cars, sizeof(num_cars));
    num_cars = big_endian_to_host_16(num_cars);

    std::vector<CarDto> cars;
    cars.reserve(num_cars);  // RAII - reserva eficiente

    for (uint16_t i = 0; i < num_cars; i++) {  // Usar uint16_t para evitar warnings
        cars.push_back(deserialize_car());
    }

    return MarketDto(cars);
}

CarPurchaseDto Protocol::deserialize_car_purchase() {
    CarDto car = deserialize_car();

    uint32_t remaining_money;
    socket.recvall(&remaining_money, sizeof(remaining_money));
    remaining_money = big_endian_to_host_32(remaining_money);

    return CarPurchaseDto(car, remaining_money);
}

ErrorDto Protocol::deserialize_error() {
    uint16_t length;
    socket.recvall(&length, sizeof(length));
    length = big_endian_to_host_16(length);

    std::string message(length, '\0');
    socket.recvall(&message[0], length);
    return ErrorDto(message);
}
