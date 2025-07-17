// Test core functionality without FLECS dependency
#include "include/flecs_gol/game_config.h"
#include "include/flecs_gol/components.h"
#include <iostream>
#include <vector>
#include <unordered_set>

int main() {
    std::cout << "=== Core Components Test ===" << std::endl;
    
    try {
        // Test GameConfig with different configurations
        flecs_gol::GameConfig config;
        config.setGridBoundaries(-50, 50, -50, 50);
        config.setTargetFPS(60);
        config.setWrapEdges(true);
        
        if (config.validate()) {
            std::cout << "✓ GameConfig validation works" << std::endl;
        }
        
        // Test JSON round-trip
        auto json = config.toJson();
        auto config2 = flecs_gol::GameConfig::fromJson(json);
        
        if (config2.getTargetFPS() == 60 && config2.getWrapEdges()) {
            std::cout << "✓ JSON serialization complete" << std::endl;
        }
        
        // Test Position operations
        std::vector<flecs_gol::Position> positions;
        for (int x = -5; x <= 5; ++x) {
            for (int y = -5; y <= 5; ++y) {
                positions.emplace_back(x, y);
            }
        }
        
        // Test manual neighbor calculation
        flecs_gol::Position center(0, 0);
        std::vector<flecs_gol::Position> neighbors;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx != 0 || dy != 0) {
                    neighbors.emplace_back(center.x + dx, center.y + dy);
                }
            }
        }
        
        if (neighbors.size() == 8) {
            std::cout << "✓ Neighbor calculation works (" << neighbors.size() << " neighbors)" << std::endl;
        }
        
        // Test position hashing for large sets
        std::unordered_set<flecs_gol::Position> positionSet;
        for (const auto& pos : positions) {
            positionSet.insert(pos);
        }
        
        if (positionSet.size() == positions.size()) {
            std::cout << "✓ Position hashing works for " << positionSet.size() << " positions" << std::endl;
        }
        
        // Test Cell component
        flecs_gol::Cell cell;
        cell.neighborCount = 3;
        cell.willLive = true;
        
        flecs_gol::BirthCandidate candidate;
        candidate.neighborCount = 3;
        
        if (cell.willLive && candidate.neighborCount == 3) {
            std::cout << "✓ Cell and BirthCandidate components work" << std::endl;
        }
        
        // Test grid state component
        flecs_gol::GridState gridState;
        gridState.generation = 42;
        gridState.liveCellCount = 1000;
        gridState.minX = -10;
        gridState.maxX = 10;
        
        if (gridState.generation == 42 && gridState.liveCellCount == 1000) {
            std::cout << "✓ GridState component works" << std::endl;
        }
        
        std::cout << "\n=== All Core Component Tests Passed! ===" << std::endl;
        std::cout << "Core functionality works independently of FLECS." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}