#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace flecs_gol {

class GameConfig {
public:
    GameConfig();
    
    // Grid boundaries
    void setGridBoundaries(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY);
    int32_t getGridMinX() const { return gridMinX_; }
    int32_t getGridMaxX() const { return gridMaxX_; }
    int32_t getGridMinY() const { return gridMinY_; }
    int32_t getGridMaxY() const { return gridMaxY_; }
    
    // Grid properties
    int32_t getGridWidth() const { return gridMaxX_ - gridMinX_ + 1; }
    int32_t getGridHeight() const { return gridMaxY_ - gridMinY_ + 1; }
    bool isPointInBounds(int32_t x, int32_t y) const;
    
    // Edge wrapping
    void setWrapEdges(bool wrap) { wrapEdges_ = wrap; }
    bool getWrapEdges() const { return wrapEdges_; }
    
    // Simulation parameters
    void setTargetFPS(uint32_t fps) { targetFPS_ = fps; }
    uint32_t getTargetFPS() const { return targetFPS_; }
    
    void setMaxGenerations(uint32_t maxGen) { maxGenerations_ = maxGen; }
    uint32_t getMaxGenerations() const { return maxGenerations_; }
    
    // Performance settings
    void setMaxEntities(uint32_t maxEntities) { maxEntities_ = maxEntities; }
    uint32_t getMaxEntities() const { return maxEntities_; }
    
    void setEnableProfiling(bool enable) { enableProfiling_ = enable; }
    bool getEnableProfiling() const { return enableProfiling_; }
    
    // Validation
    bool validate() const;
    
    // JSON serialization
    nlohmann::json toJson() const;
    static GameConfig fromJson(const nlohmann::json& json);
    
    // File I/O
    bool saveToFile(const std::string& filename) const;
    static std::optional<GameConfig> loadFromFile(const std::string& filename);

private:
    // Grid boundaries
    int32_t gridMinX_ = -500;
    int32_t gridMaxX_ = 500;
    int32_t gridMinY_ = -500;
    int32_t gridMaxY_ = 500;
    bool wrapEdges_ = false;
    
    // Simulation parameters
    uint32_t targetFPS_ = 10;
    uint32_t maxGenerations_ = 0; // 0 = unlimited
    
    // Performance settings
    uint32_t maxEntities_ = 1000000;
    bool enableProfiling_ = false;
};

} // namespace flecs_gol