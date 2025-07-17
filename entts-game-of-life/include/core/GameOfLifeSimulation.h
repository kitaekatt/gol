#pragma once

#include "GameConfig.h"
#include "components/Position.h"
#include "components/Cell.h"
#include <entt/entt.hpp>
#include <unordered_map>
#include <array>
#include <cstdint>

class GameOfLifeSimulation {
public:
    explicit GameOfLifeSimulation(const GameConfig& config = GameConfig{});
    
    // Cell manipulation
    void setCellAlive(std::int32_t x, std::int32_t y);
    void setCellDead(std::int32_t x, std::int32_t y);
    bool isCellAlive(std::int32_t x, std::int32_t y) const;
    
    // Simulation control
    bool step(); // Returns true if changes occurred
    void reset();
    
    // State queries
    std::size_t getLivingCellCount() const;
    std::uint8_t getNeighborCount(std::int32_t x, std::int32_t y) const;
    std::uint64_t getGenerationCount() const { return generationCount_; }
    
    // Entity access (for testing)
    entt::entity getEntityAt(std::int32_t x, std::int32_t y) const;
    const entt::registry& getRegistry() const { return registry_; }
    
    // Configuration
    const GameConfig& getConfig() const { return config_; }
    void setConfig(const GameConfig& config);

private:
    GameConfig config_;
    entt::registry registry_;
    std::unordered_map<Position, entt::entity> spatialIndex_;
    std::uint64_t generationCount_{0};
    
    // Helper methods
    bool isValidPosition(std::int32_t x, std::int32_t y) const;
    Position normalizePosition(std::int32_t x, std::int32_t y) const;
    std::uint8_t calculateNeighborCount(std::int32_t x, std::int32_t y) const;
    void updateNeighborCounts();
    void applyConwayRules();
    void cleanupDeadCells();
    
    // Neighbor position offsets
    static constexpr std::array<std::pair<std::int32_t, std::int32_t>, 8> neighborOffsets{{
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    }};
};