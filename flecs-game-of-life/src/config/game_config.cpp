#include <flecs_gol/game_config.h>
#include <fstream>
#include <iostream>

namespace flecs_gol {

GameConfig::GameConfig() {
    // Default values are set in header
}

void GameConfig::setGridBoundaries(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) {
    gridMinX_ = minX;
    gridMaxX_ = maxX;
    gridMinY_ = minY;
    gridMaxY_ = maxY;
}

bool GameConfig::isPointInBounds(int32_t x, int32_t y) const {
    return x >= gridMinX_ && x <= gridMaxX_ && y >= gridMinY_ && y <= gridMaxY_;
}

bool GameConfig::validate() const {
    // Check grid boundaries
    if (gridMinX_ >= gridMaxX_ || gridMinY_ >= gridMaxY_) {
        return false;
    }
    
    // Check simulation parameters
    if (targetFPS_ == 0) {
        return false;
    }
    
    if (maxEntities_ == 0) {
        return false;
    }
    
    return true;
}

nlohmann::json GameConfig::toJson() const {
    nlohmann::json json;
    
    // Grid configuration
    json["grid"]["boundaries"]["minX"] = gridMinX_;
    json["grid"]["boundaries"]["maxX"] = gridMaxX_;
    json["grid"]["boundaries"]["minY"] = gridMinY_;
    json["grid"]["boundaries"]["maxY"] = gridMaxY_;
    json["grid"]["wrapEdges"] = wrapEdges_;
    
    // Simulation configuration
    json["simulation"]["targetFPS"] = targetFPS_;
    json["simulation"]["maxGenerations"] = maxGenerations_;
    
    // Performance configuration
    json["performance"]["maxEntities"] = maxEntities_;
    json["performance"]["enableProfiling"] = enableProfiling_;
    
    return json;
}

GameConfig GameConfig::fromJson(const nlohmann::json& json) {
    GameConfig config;
    
    // Grid boundaries
    if (json.contains("grid") && json["grid"].contains("boundaries")) {
        const auto& boundaries = json["grid"]["boundaries"];
        if (boundaries.contains("minX")) config.gridMinX_ = boundaries["minX"];
        if (boundaries.contains("maxX")) config.gridMaxX_ = boundaries["maxX"];
        if (boundaries.contains("minY")) config.gridMinY_ = boundaries["minY"];
        if (boundaries.contains("maxY")) config.gridMaxY_ = boundaries["maxY"];
    }
    
    if (json.contains("grid") && json["grid"].contains("wrapEdges")) {
        config.wrapEdges_ = json["grid"]["wrapEdges"];
    }
    
    // Simulation parameters
    if (json.contains("simulation")) {
        const auto& simulation = json["simulation"];
        if (simulation.contains("targetFPS")) config.targetFPS_ = simulation["targetFPS"];
        if (simulation.contains("maxGenerations")) config.maxGenerations_ = simulation["maxGenerations"];
    }
    
    // Performance settings
    if (json.contains("performance")) {
        const auto& performance = json["performance"];
        if (performance.contains("maxEntities")) config.maxEntities_ = performance["maxEntities"];
        if (performance.contains("enableProfiling")) config.enableProfiling_ = performance["enableProfiling"];
    }
    
    return config;
}

bool GameConfig::saveToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json json = toJson();
        file << json.dump(4);
        file.close();
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::optional<GameConfig> GameConfig::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        nlohmann::json json;
        file >> json;
        file.close();
        
        return fromJson(json);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace flecs_gol