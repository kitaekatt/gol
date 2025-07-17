// Test pattern loading and JSON parsing
#include "include/flecs_gol/game_config.h"
#include "include/flecs_gol/components.h"
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Pattern {
    std::string name;
    std::vector<flecs_gol::Position> cells;
    int width, height;
    flecs_gol::Position initialPosition;
    
    static Pattern fromJson(const json& j) {
        Pattern pattern;
        pattern.name = j["metadata"]["name"];
        pattern.width = j["boundingBox"]["width"];
        pattern.height = j["boundingBox"]["height"];
        
        if (j.contains("initialPosition")) {
            pattern.initialPosition.x = j["initialPosition"]["x"];
            pattern.initialPosition.y = j["initialPosition"]["y"];
        }
        
        for (const auto& cell : j["cells"]) {
            pattern.cells.emplace_back(
                cell["x"].get<int>() + pattern.initialPosition.x,
                cell["y"].get<int>() + pattern.initialPosition.y
            );
        }
        
        return pattern;
    }
};

int main() {
    std::cout << "=== Pattern Loading Test ===" << std::endl;
    
    try {
        // Test glider pattern
        std::ifstream gliderFile("examples/patterns/glider.json");
        if (!gliderFile.is_open()) {
            std::cerr << "✗ Could not open glider.json" << std::endl;
            return 1;
        }
        
        json gliderJson;
        gliderFile >> gliderJson;
        
        auto glider = Pattern::fromJson(gliderJson);
        
        if (glider.name == "Glider" && glider.cells.size() == 5) {
            std::cout << "✓ Glider pattern loaded: " << glider.cells.size() << " cells" << std::endl;
        } else {
            std::cout << "✗ Glider pattern failed: got " << glider.cells.size() << " cells" << std::endl;
            return 1;
        }
        
        // Test blinker pattern
        std::ifstream blinkerFile("examples/patterns/blinker.json");
        if (!blinkerFile.is_open()) {
            std::cerr << "✗ Could not open blinker.json" << std::endl;
            return 1;
        }
        
        json blinkerJson;
        blinkerFile >> blinkerJson;
        
        auto blinker = Pattern::fromJson(blinkerJson);
        
        if (blinker.name == "Blinker" && blinker.cells.size() == 3) {
            std::cout << "✓ Blinker pattern loaded: " << blinker.cells.size() << " cells" << std::endl;
        } else {
            std::cout << "✗ Blinker pattern failed: got " << blinker.cells.size() << " cells" << std::endl;
            return 1;
        }
        
        // Test configuration with pattern
        flecs_gol::GameConfig config;
        config.setGridBoundaries(-20, 20, -20, 20);
        config.setTargetFPS(30);
        
        // Verify all glider cells are within bounds
        bool allWithinBounds = true;
        for (const auto& pos : glider.cells) {
            if (pos.x < -20 || pos.x > 20 || pos.y < -20 || pos.y > 20) {
                allWithinBounds = false;
                break;
            }
        }
        
        if (allWithinBounds) {
            std::cout << "✓ Pattern cells are within configured grid bounds" << std::endl;
        } else {
            std::cout << "✗ Some pattern cells are outside grid bounds" << std::endl;
            return 1;
        }
        
        // Test pattern position conversion
        std::unordered_set<flecs_gol::Position> positionSet;
        for (const auto& pos : glider.cells) {
            positionSet.insert(pos);
        }
        
        if (positionSet.size() == glider.cells.size()) {
            std::cout << "✓ Pattern position uniqueness verified" << std::endl;
        } else {
            std::cout << "✗ Pattern has duplicate positions" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== All Pattern Tests Passed! ===" << std::endl;
        std::cout << "Pattern loading and JSON parsing work correctly." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Pattern test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}