// Simple build test - compile this first to check basic setup
#include "core/GameConfig.h"
#include "core/GameOfLifeSimulation.h"
#include <iostream>

int main() {
    try {
        // Test basic configuration
        GameConfig config;
        config.setGridWidth(10);
        config.setGridHeight(10);
        
        std::cout << "GameConfig created successfully" << std::endl;
        std::cout << "Grid size: " << config.getGridWidth() << "x" << config.getGridHeight() << std::endl;
        
        // Test simulation creation
        GameOfLifeSimulation simulation(config);
        std::cout << "GameOfLifeSimulation created successfully" << std::endl;
        std::cout << "Initial living cells: " << simulation.getLivingCellCount() << std::endl;
        
        // Test basic cell operations
        simulation.setCellAlive(5, 5);
        simulation.setCellAlive(5, 6);
        simulation.setCellAlive(6, 5);
        
        std::cout << "Cells added. Living cells: " << simulation.getLivingCellCount() << std::endl;
        
        // Test one simulation step
        simulation.step();
        std::cout << "Step completed. Generation: " << simulation.getGenerationCount() << std::endl;
        std::cout << "Living cells after step: " << simulation.getLivingCellCount() << std::endl;
        
        std::cout << "✅ Basic build test PASSED!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}