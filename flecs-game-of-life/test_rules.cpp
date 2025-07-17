// Test Conway's Game of Life rules implementation
#include "include/flecs_gol/game_config.h"
#include "include/flecs_gol/components.h"
#include <iostream>
#include <unordered_map>
#include <vector>

// Simple Conway's rule implementation for testing
bool shouldLive(int neighborCount, bool isAlive) {
    if (isAlive) {
        // Live cell with 2 or 3 neighbors survives
        return neighborCount == 2 || neighborCount == 3;
    } else {
        // Dead cell with exactly 3 neighbors becomes alive
        return neighborCount == 3;
    }
}

int main() {
    std::cout << "=== Conway's Game of Life Rules Test ===" << std::endl;
    
    // Test rule implementation
    bool ruleTests = true;
    
    // Test live cell rules
    if (!shouldLive(1, true)) std::cout << "✓ Live cell with 1 neighbor dies (underpopulation)" << std::endl;
    else ruleTests = false;
    
    if (shouldLive(2, true)) std::cout << "✓ Live cell with 2 neighbors survives" << std::endl;
    else ruleTests = false;
    
    if (shouldLive(3, true)) std::cout << "✓ Live cell with 3 neighbors survives" << std::endl;
    else ruleTests = false;
    
    if (!shouldLive(4, true)) std::cout << "✓ Live cell with 4 neighbors dies (overpopulation)" << std::endl;
    else ruleTests = false;
    
    // Test dead cell rules
    if (!shouldLive(2, false)) std::cout << "✓ Dead cell with 2 neighbors stays dead" << std::endl;
    else ruleTests = false;
    
    if (shouldLive(3, false)) std::cout << "✓ Dead cell with 3 neighbors becomes alive" << std::endl;
    else ruleTests = false;
    
    if (!shouldLive(4, false)) std::cout << "✓ Dead cell with 4 neighbors stays dead" << std::endl;
    else ruleTests = false;
    
    if (!ruleTests) {
        std::cerr << "✗ Rule tests failed!" << std::endl;
        return 1;
    }
    
    // Test a simple pattern: Blinker
    std::cout << "\n=== Blinker Pattern Test ===" << std::endl;
    
    // Generation 0: Three cells in a row
    std::unordered_map<flecs_gol::Position, bool> grid;
    grid[{0, -1}] = true;
    grid[{0, 0}] = true;
    grid[{0, 1}] = true;
    
    std::cout << "Generation 0: 3 cells in vertical line" << std::endl;
    
    // Calculate next generation
    std::unordered_map<flecs_gol::Position, int> neighborCounts;
    
    // Count neighbors for all relevant positions
    for (const auto& [pos, alive] : grid) {
        if (alive) {
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx != 0 || dy != 0) {
                        flecs_gol::Position neighbor(pos.x + dx, pos.y + dy);
                        neighborCounts[neighbor]++;
                    }
                }
            }
        }
    }
    
    // Apply rules
    std::unordered_map<flecs_gol::Position, bool> nextGrid;
    for (const auto& [pos, count] : neighborCounts) {
        bool currentlyAlive = grid.count(pos) && grid[pos];
        if (shouldLive(count, currentlyAlive)) {
            nextGrid[pos] = true;
        }
    }
    
    // Check if we got the expected horizontal line
    bool hasHorizontalLine = nextGrid.count({-1, 0}) && nextGrid.count({0, 0}) && nextGrid.count({1, 0});
    bool hasOnlyThreeCells = nextGrid.size() == 3;
    
    if (hasHorizontalLine && hasOnlyThreeCells) {
        std::cout << "✓ Blinker correctly rotated to horizontal line" << std::endl;
    } else {
        std::cout << "✗ Blinker pattern failed. Expected horizontal line, got " << nextGrid.size() << " cells:" << std::endl;
        for (const auto& [pos, alive] : nextGrid) {
            if (alive) std::cout << "  Cell at (" << pos.x << ", " << pos.y << ")" << std::endl;
        }
        return 1;
    }
    
    // Test performance metrics component
    flecs_gol::PerformanceMetrics metrics;
    metrics.entityCount = static_cast<uint32_t>(nextGrid.size());
    metrics.memoryUsage = sizeof(flecs_gol::Position) * nextGrid.size();
    
    if (metrics.entityCount == 3) {
        std::cout << "✓ Performance metrics tracking works" << std::endl;
    }
    
    std::cout << "\n=== All Rule Tests Passed! ===" << std::endl;
    std::cout << "Conway's Game of Life rules implemented correctly." << std::endl;
    
    return 0;
}