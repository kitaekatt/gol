#pragma once

#include <cstdint>

struct Cell {
    bool alive{true};
    std::uint8_t neighborCount{0};
    
    constexpr Cell() noexcept = default;
    constexpr Cell(bool isAlive) noexcept : alive(isAlive) {}
    constexpr Cell(bool isAlive, std::uint8_t neighbors) noexcept 
        : alive(isAlive), neighborCount(neighbors) {}
    
    constexpr bool operator==(const Cell& other) const noexcept {
        return alive == other.alive && neighborCount == other.neighborCount;
    }
};