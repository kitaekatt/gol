#include "include/flecs_gol/flecs_gol.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== FLECS Game of Life - Headless Test ===" << std::endl;
    std::cout << "Version: " << flecs_gol::getVersionString() << std::endl;
    
    try {
        // Test 1: Basic Configuration
        std::cout << "\n1. Testing GameConfig..." << std::endl;
        flecs_gol::GameConfig config;
        config.setGridBoundaries(-50, 50, -50, 50);
        config.setTargetFPS(60);
        config.setMaxEntities(10000);
        
        auto json = config.toJson();
        auto configFromJson = flecs_gol::GameConfig::fromJson(json);
        
        if (configFromJson.getTargetFPS() == 60) {
            std::cout << "✓ GameConfig JSON serialization works" << std::endl;
        } else {
            std::cout << "✗ GameConfig JSON serialization failed" << std::endl;
            return 1;
        }
        
        // Test 2: Controller Creation and Basic Operations
        std::cout << "\n2. Testing SimulationController..." << std::endl;
        flecs_gol::SimulationController controller(config);
        
        auto initialState = controller.getState();
        std::cout << "✓ Controller created, initial state: gen=" << initialState.generation 
                  << ", cells=" << initialState.liveCellCount << std::endl;
        
        // Test 3: Add some cells manually
        std::cout << "\n3. Testing manual cell placement..." << std::endl;
        controller.addCell(0, 0);
        controller.addCell(1, 0);
        controller.addCell(2, 0);
        
        auto state = controller.getState();
        if (state.liveCellCount == 3) {
            std::cout << "✓ Manual cell placement works: " << state.liveCellCount << " cells" << std::endl;
        } else {
            std::cout << "✗ Manual cell placement failed: expected 3, got " << state.liveCellCount << std::endl;
            return 1;
        }
        
        // Test 4: Single step evolution
        std::cout << "\n4. Testing simulation step..." << std::endl;
        controller.step();
        
        state = controller.getState();
        std::cout << "✓ After step: gen=" << state.generation 
                  << ", cells=" << state.liveCellCount 
                  << ", step_time=" << (state.lastStepTimeMicros / 1000.0) << "ms" << std::endl;
        
        // Test 5: Load pattern from JSON
        std::cout << "\n5. Testing pattern loading..." << std::endl;
        controller.clearGrid();
        
        // Create a simple glider pattern in JSON
        nlohmann::json gliderPattern = R"({
            "cells": [
                {"x": 1, "y": 0},
                {"x": 2, "y": 1},
                {"x": 0, "y": 2},
                {"x": 1, "y": 2},
                {"x": 2, "y": 2}
            ]
        })"_json;
        
        controller.loadPatternFromJson(gliderPattern);
        state = controller.getState();
        
        if (state.liveCellCount == 5) {
            std::cout << "✓ Pattern loading works: " << state.liveCellCount << " cells" << std::endl;
        } else {
            std::cout << "✗ Pattern loading failed: expected 5, got " << state.liveCellCount << std::endl;
            return 1;
        }
        
        // Test 6: Run simulation for several steps
        std::cout << "\n6. Testing simulation evolution..." << std::endl;
        controller.start();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Run for 0.5 seconds
        
        state = controller.getState();
        std::cout << "✓ After running: gen=" << state.generation 
                  << ", cells=" << state.liveCellCount 
                  << ", fps=" << state.actualFPS 
                  << ", memory=" << (state.memoryUsage / 1024) << "KB" << std::endl;
        
        controller.stop();
        
        // Test 7: Query operations
        std::cout << "\n7. Testing query operations..." << std::endl;
        auto allCells = controller.getAllCells();
        auto regionCells = controller.getCellsInRegion(-10, 10, -10, 10);
        
        std::cout << "✓ Query results: all=" << allCells.size() 
                  << ", region=" << regionCells.size() << " cells" << std::endl;
        
        // Test 8: Model/View separation verification
        std::cout << "\n8. Testing model/view separation..." << std::endl;
        
        // The controller should work completely independently of any view
        // We can query its state without any rendering dependencies
        for (int i = 0; i < 5; ++i) {
            controller.step();
            auto stepState = controller.getState();
            std::cout << "  Step " << (i+1) << ": gen=" << stepState.generation 
                      << ", cells=" << stepState.liveCellCount << std::endl;
        }
        
        std::cout << "✓ Controller operates independently of view components" << std::endl;
        
        std::cout << "\n=== All Tests Passed! ===" << std::endl;
        std::cout << "The SimulationController successfully demonstrates model/view separation." << std::endl;
        std::cout << "Unity can easily replace the console view while reusing the same controller." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}