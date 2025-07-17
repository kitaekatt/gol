#include "core/GameConfig.h"
#include <fstream>
#include <stdexcept>

GameConfig::GameConfig() {
    setDefaults();
}

nlohmann::json GameConfig::toJson() const {
    nlohmann::json json;
    
    json["grid"]["width"] = gridWidth_;
    json["grid"]["height"] = gridHeight_;
    json["grid"]["wrap_edges"] = wrapEdges_;
    
    json["simulation"]["max_generations"] = maxGenerations_;
    json["simulation"]["auto_pause_on_stable"] = autoPauseOnStable_;
    json["simulation"]["stable_detection_cycles"] = stableDetectionCycles_;
    json["simulation"]["step_delay_ms"] = stepDelayMs_;
    
    json["performance"]["target_fps"] = targetFps_;
    json["performance"]["memory_limit_mb"] = memoryLimitMb_;
    json["performance"]["enable_spatial_optimization"] = enableSpatialOptimization_;
    json["performance"]["batch_size"] = batchSize_;
    
    return json;
}

void GameConfig::fromJson(const nlohmann::json& json) {
    // Grid settings
    if (json.contains("grid")) {
        const auto& grid = json["grid"];
        if (grid.contains("width")) {
            gridWidth_ = grid["width"];
        }
        if (grid.contains("height")) {
            gridHeight_ = grid["height"];
        }
        if (grid.contains("wrap_edges")) {
            wrapEdges_ = grid["wrap_edges"];
        }
    }
    
    // Simulation settings
    if (json.contains("simulation")) {
        const auto& simulation = json["simulation"];
        if (simulation.contains("max_generations")) {
            maxGenerations_ = simulation["max_generations"];
        }
        if (simulation.contains("auto_pause_on_stable")) {
            autoPauseOnStable_ = simulation["auto_pause_on_stable"];
        }
        if (simulation.contains("stable_detection_cycles")) {
            stableDetectionCycles_ = simulation["stable_detection_cycles"];
        }
        if (simulation.contains("step_delay_ms")) {
            stepDelayMs_ = simulation["step_delay_ms"];
        }
    }
    
    // Performance settings
    if (json.contains("performance")) {
        const auto& performance = json["performance"];
        if (performance.contains("target_fps")) {
            targetFps_ = performance["target_fps"];
        }
        if (performance.contains("memory_limit_mb")) {
            memoryLimitMb_ = performance["memory_limit_mb"];
        }
        if (performance.contains("enable_spatial_optimization")) {
            enableSpatialOptimization_ = performance["enable_spatial_optimization"];
        }
        if (performance.contains("batch_size")) {
            batchSize_ = performance["batch_size"];
        }
    }
}

void GameConfig::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    
    file << toJson().dump(2);
    file.close();
}

void GameConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + filename);
    }
    
    nlohmann::json json;
    file >> json;
    file.close();
    
    fromJson(json);
}

bool GameConfig::isValid() const {
    // Grid validation
    if (gridWidth_ <= 0 || gridHeight_ <= 0) {
        return false;
    }
    
    // Simulation validation
    if (maxGenerations_ < 0) {
        return false;
    }
    if (stableDetectionCycles_ < 0) {
        return false;
    }
    if (stepDelayMs_ < 0) {
        return false;
    }
    
    // Performance validation
    if (targetFps_ <= 0) {
        return false;
    }
    if (memoryLimitMb_ < 0) {
        return false;
    }
    if (batchSize_ <= 0) {
        return false;
    }
    
    return true;
}

void GameConfig::setDefaults() {
    gridWidth_ = 100;
    gridHeight_ = 100;
    wrapEdges_ = false;
    
    maxGenerations_ = 1000;
    autoPauseOnStable_ = true;
    stableDetectionCycles_ = 10;
    stepDelayMs_ = 100;
    
    targetFps_ = 60;
    memoryLimitMb_ = 100;
    enableSpatialOptimization_ = true;
    batchSize_ = 1000;
}