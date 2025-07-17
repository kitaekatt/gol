#pragma once

#include <cstdint>
#include <cstddef>

namespace flecs_gol {

struct Position {
    int32_t x;
    int32_t y;
    
    Position() : x(0), y(0) {}
    Position(int32_t x, int32_t y) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Position& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
    
    // Hash function for spatial indexing
    size_t hash() const {
        return (static_cast<size_t>(x) << 32) | static_cast<size_t>(y);
    }
};

struct Cell {
    uint8_t neighborCount = 0;
    bool willLive = false;  // Computed during rule evaluation phase
    
    Cell() = default;
    Cell(uint8_t neighbors) : neighborCount(neighbors), willLive(false) {}
};

// Optional component for cells that are about to be born
struct BirthCandidate {
    uint8_t neighborCount = 0;
    
    BirthCandidate() = default;
    BirthCandidate(uint8_t neighbors) : neighborCount(neighbors) {}
};

// Grid metadata (singleton component)
struct GridState {
    uint32_t generation = 0;
    uint32_t liveCellCount = 0;
    int32_t minX = 0, maxX = 0, minY = 0, maxY = 0; // Current bounds of active area
    
    GridState() = default;
};

// Performance metrics (singleton component)
struct PerformanceMetrics {
    size_t memoryUsage = 0;
    uint32_t entityCount = 0;
    uint64_t lastUpdateTimeMicros = 0;
    uint64_t neighborCountTimeMicros = 0;
    uint64_t ruleEvalTimeMicros = 0;
    uint64_t lifecycleTimeMicros = 0;
    
    PerformanceMetrics() = default;
};

} // namespace flecs_gol

// Hash specialization for Position to enable use in unordered containers
namespace std {
    template<>
    struct hash<flecs_gol::Position> {
        size_t operator()(const flecs_gol::Position& pos) const {
            return pos.hash();
        }
    };
}