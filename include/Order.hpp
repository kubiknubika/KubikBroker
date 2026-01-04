#pragma once
#include <cstdint>

enum class Side : uint8_t { Buy, Sell };

struct Order {
    uint64_t id;
    uint64_t price;
    uint64_t quantity;
    Side side;
    // Padding не нужен, структура компактная (8+8+8+1 = 25 байт -> 32 байта с выравниванием)
};