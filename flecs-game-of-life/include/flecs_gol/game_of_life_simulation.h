#pragma once

#include <flecs.h>
#include <flecs_gol/game_config.h>
#include <flecs_gol/components.h>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace flecs_gol {

class GameOfLifeSimulation {
public:
    explicit GameOfLifeSimulation(const GameConfig& config);
    ~GameOfLifeSimulation() = default;
    
    // Entity management
    flecs::entity createCell(int32_t x, int32_t y);
    void destroyCell(int32_t x, int32_t y);
    bool isCellAlive(int32_t x, int32_t y) const;
    flecs::entity getCellAt(int32_t x, int32_t y) const;
    
    // Simulation control
    void step();
    void reset();
    void clear();
    
    // State queries
    uint32_t getCellCount() const;
    uint32_t getGeneration() const;
    size_t getMemoryUsage() const;
    
    // Neighbor operations
    uint8_t getNeighborCount(int32_t x, int32_t y) const;
    void updateNeighborCounts();
    
    // Query operations
    std::vector<flecs::entity> getAllCells() const;
    std::vector<flecs::entity> getCellsInRegion(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) const;
    std::vector<flecs::entity> getCellsWithNeighborCount(uint8_t count) const;
    
    // Configuration
    const GameConfig& getConfig() const { return config_; }

private:
    // Internal systems
    void neighborCountSystem();
    void ruleEvaluationSystem();
    void lifecycleSystem();
    void updatePerformanceMetrics();
    
    // Utility methods
    bool isValidPosition(int32_t x, int32_t y) const;
    Position wrapPosition(int32_t x, int32_t y) const;
    std::vector<Position> getNeighborPositions(int32_t x, int32_t y) const;
    void rebuildSpatialIndex();
    
    // Data members
    flecs::world world_;
    GameConfig config_;
    
    // Spatial indexing for fast position lookups
    std::unordered_map<Position, flecs::entity> spatialIndex_;
    
    // Singleton entities
    flecs::entity gridStateEntity_;
    flecs::entity performanceEntity_;
    
    // Cached queries for performance
    flecs::query<Position, Cell> liveCellQuery_;
    flecs::query<Position, BirthCandidate> birthCandidateQuery_;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastStepTime_;
    
    static constexpr std::array<std::pair<int32_t, int32_t>, 8> NEIGHBOR_OFFSETS = {{
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    }};
};

} // namespace flecs_gol