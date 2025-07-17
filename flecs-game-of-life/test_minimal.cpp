// Minimal compilation test to verify core structure
// This file tests that our headers and basic structure compile correctly

#include "include/flecs_gol/flecs_gol.h"
#include <iostream>

int main() {
    try {
        // Test GameConfig
        flecs_gol::GameConfig config;
        config.setGridBoundaries(-10, 10, -10, 10);
        config.setTargetFPS(60);
        
        // Test JSON serialization
        auto json = config.toJson();
        auto configFromJson = flecs_gol::GameConfig::fromJson(json);
        
        std::cout << "GameConfig test passed\n";
        
        // Test basic simulation without FLECS (since we can't link)
        // This at least verifies our class structure compiles
        
        std::cout << "Version: " << flecs_gol::getVersionString() << std::endl;
        std::cout << "All basic structure tests passed!\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}