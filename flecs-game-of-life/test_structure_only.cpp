// Test basic structure compilation without FLECS dependency
#include "include/flecs_gol/game_config.h"
#include "include/flecs_gol/components.h"
#include <iostream>

int main() {
    std::cout << "=== Structure Test (No FLECS) ===" << std::endl;
    
    try {
        // Test GameConfig
        flecs_gol::GameConfig config;
        config.setGridBoundaries(-10, 10, -10, 10);
        config.setTargetFPS(30);
        
        if (config.validate()) {
            std::cout << "✓ GameConfig creation and validation works" << std::endl;
        } else {
            std::cout << "✗ GameConfig validation failed" << std::endl;
            return 1;
        }
        
        // Test JSON serialization
        auto json = config.toJson();
        auto configFromJson = flecs_gol::GameConfig::fromJson(json);
        
        if (configFromJson.getTargetFPS() == 30) {
            std::cout << "✓ GameConfig JSON serialization works" << std::endl;
        } else {
            std::cout << "✗ GameConfig JSON serialization failed" << std::endl;
            return 1;
        }
        
        // Test components structure
        flecs_gol::Position pos(5, 10);
        flecs_gol::Cell cell;
        cell.neighborCount = 3;
        cell.willLive = true;
        
        if (pos.x == 5 && pos.y == 10 && cell.neighborCount == 3) {
            std::cout << "✓ Component structures work correctly" << std::endl;
        } else {
            std::cout << "✗ Component structures failed" << std::endl;
            return 1;
        }
        
        // Test position hashing
        auto hash1 = pos.hash();
        flecs_gol::Position pos2(5, 10);
        auto hash2 = pos2.hash();
        
        if (hash1 == hash2) {
            std::cout << "✓ Position hashing works" << std::endl;
        } else {
            std::cout << "✗ Position hashing failed" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== Basic Structure Test Passed! ===" << std::endl;
        std::cout << "Core structures compile and work without FLECS dependency." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}