#pragma once

#include <cstdint>
#include <functional>

struct Position {
    std::int32_t x{0};
    std::int32_t y{0};
    
    constexpr Position() noexcept = default;
    constexpr Position(std::int32_t x_val, std::int32_t y_val) noexcept 
        : x(x_val), y(y_val) {}
    
    constexpr bool operator==(const Position& other) const noexcept {
        return x == other.x && y == other.y;
    }
    
    constexpr auto operator<=>(const Position& other) const noexcept {
        if (auto cmp = x <=> other.x; cmp != 0) {
            return cmp;
        }
        return y <=> other.y;
    }
};

// Hash specialization for Position to use in unordered containers
namespace std {
    template<>
    struct hash<Position> {
        std::size_t operator()(const Position& pos) const noexcept {
            // Simple hash combining x and y coordinates
            return std::hash<std::int32_t>{}(pos.x) ^ 
                   (std::hash<std::int32_t>{}(pos.y) << 1);
        }
    };
}