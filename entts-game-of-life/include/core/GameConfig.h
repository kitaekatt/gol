#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <cstdint>

class GameConfig {
public:
    GameConfig();
    
    // Grid settings
    std::int32_t getGridWidth() const { return gridWidth_; }
    std::int32_t getGridHeight() const { return gridHeight_; }
    bool getWrapEdges() const { return wrapEdges_; }
    
    void setGridWidth(std::int32_t width) { gridWidth_ = width; }
    void setGridHeight(std::int32_t height) { gridHeight_ = height; }
    void setWrapEdges(bool wrap) { wrapEdges_ = wrap; }
    
    // Simulation settings
    std::int32_t getMaxGenerations() const { return maxGenerations_; }
    bool getAutoPauseOnStable() const { return autoPauseOnStable_; }
    std::int32_t getStableDetectionCycles() const { return stableDetectionCycles_; }
    std::int32_t getStepDelayMs() const { return stepDelayMs_; }
    
    void setMaxGenerations(std::int32_t maxGen) { maxGenerations_ = maxGen; }
    void setAutoPauseOnStable(bool autoPause) { autoPauseOnStable_ = autoPause; }
    void setStableDetectionCycles(std::int32_t cycles) { stableDetectionCycles_ = cycles; }
    void setStepDelayMs(std::int32_t delay) { stepDelayMs_ = delay; }
    
    // Performance settings
    std::int32_t getTargetFps() const { return targetFps_; }
    std::int32_t getMemoryLimitMb() const { return memoryLimitMb_; }
    bool getEnableSpatialOptimization() const { return enableSpatialOptimization_; }
    std::int32_t getBatchSize() const { return batchSize_; }
    
    void setTargetFps(std::int32_t fps) { targetFps_ = fps; }
    void setMemoryLimitMb(std::int32_t limitMb) { memoryLimitMb_ = limitMb; }
    void setEnableSpatialOptimization(bool enable) { enableSpatialOptimization_ = enable; }
    void setBatchSize(std::int32_t size) { batchSize_ = size; }
    
    // JSON serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
    
    // File operations
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
    // Validation
    bool isValid() const;
    
private:
    // Grid settings
    std::int32_t gridWidth_{100};
    std::int32_t gridHeight_{100};
    bool wrapEdges_{false};
    
    // Simulation settings
    std::int32_t maxGenerations_{1000};
    bool autoPauseOnStable_{true};
    std::int32_t stableDetectionCycles_{10};
    std::int32_t stepDelayMs_{100};
    
    // Performance settings
    std::int32_t targetFps_{60};
    std::int32_t memoryLimitMb_{100};
    bool enableSpatialOptimization_{true};
    std::int32_t batchSize_{1000};
    
    void setDefaults();
};