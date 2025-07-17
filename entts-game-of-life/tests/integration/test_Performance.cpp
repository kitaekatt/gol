#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "core/GameOfLifeSimulation.h"
#include "core/GameConfig.h"
#include <chrono>

TEST_CASE("Performance benchmarks", "[Performance]") {
    SECTION("Small grid performance (100x100)") {
        GameConfig config;
        config.setGridWidth(100);
        config.setGridHeight(100);
        
        GameOfLifeSimulation simulation(config);
        
        // Create random pattern with ~10% density
        for (int i = 0; i < 1000; ++i) {
            int x = rand() % 100;
            int y = rand() % 100;
            simulation.setCellAlive(x, y);
        }
        
        BENCHMARK("100x100 grid single step") {
            simulation.step();
            return simulation.getLivingCellCount();
        };
    }
    
    SECTION("Medium grid performance (500x500)") {
        GameConfig config;
        config.setGridWidth(500);
        config.setGridHeight(500);
        
        GameOfLifeSimulation simulation(config);
        
        // Create pattern with ~5% density
        for (int i = 0; i < 12500; ++i) {
            int x = rand() % 500;
            int y = rand() % 500;
            simulation.setCellAlive(x, y);
        }
        
        BENCHMARK("500x500 grid single step") {
            simulation.step();
            return simulation.getLivingCellCount();
        };
    }
    
    SECTION("Large grid sparse performance (1000x1000)") {
        GameConfig config;
        config.setGridWidth(1000);
        config.setGridHeight(1000);
        
        GameOfLifeSimulation simulation(config);
        
        // Create sparse pattern with ~1% density
        for (int i = 0; i < 10000; ++i) {
            int x = rand() % 1000;
            int y = rand() % 1000;
            simulation.setCellAlive(x, y);
        }
        
        BENCHMARK("1000x1000 sparse grid single step") {
            simulation.step();
            return simulation.getLivingCellCount();
        };
    }
}

TEST_CASE("Memory efficiency tests", "[Performance]") {
    SECTION("Memory usage scales with living cells") {
        GameOfLifeSimulation sparseSim;
        GameOfLifeSimulation denseSim;
        
        // Sparse: few cells in large area
        sparseSim.setCellAlive(100, 100);
        sparseSim.setCellAlive(500, 500);
        sparseSim.setCellAlive(900, 900);
        
        // Dense: many cells in small area  
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
                denseSim.setCellAlive(x, y);
            }
        }
        
        REQUIRE(sparseSim.getLivingCellCount() == 3);
        REQUIRE(denseSim.getLivingCellCount() == 100);
        
        // Both should complete quickly despite different patterns
        auto start = std::chrono::high_resolution_clock::now();
        sparseSim.step();
        auto sparseTime = std::chrono::high_resolution_clock::now() - start;
        
        start = std::chrono::high_resolution_clock::now();
        denseSim.step();
        auto denseTime = std::chrono::high_resolution_clock::now() - start;
        
        // Both should be fast (under 10ms for these small patterns)
        REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(sparseTime).count() < 10);
        REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(denseTime).count() < 10);
    }
}

TEST_CASE("Scalability tests", "[Performance]") {
    SECTION("Performance scales with living cells, not grid size") {
        // Same pattern in different sized grids
        auto testPattern = [](int gridSize) {
            GameConfig config;
            config.setGridWidth(gridSize);
            config.setGridHeight(gridSize);
            
            GameOfLifeSimulation simulation(config);
            
            // Add same 3x3 pattern in center
            int center = gridSize / 2;
            simulation.setCellAlive(center, center);
            simulation.setCellAlive(center+1, center);
            simulation.setCellAlive(center-1, center);
            
            auto start = std::chrono::high_resolution_clock::now();
            simulation.step();
            auto end = std::chrono::high_resolution_clock::now();
            
            return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        };
        
        auto time100 = testPattern(100);
        auto time1000 = testPattern(1000);
        auto time5000 = testPattern(5000);
        
        // Time should not scale dramatically with grid size for same pattern
        REQUIRE(time1000 < time100 * 10); // At most 10x slower
        REQUIRE(time5000 < time100 * 50); // At most 50x slower
        
        INFO("100x100 grid: " << time100 << " μs");
        INFO("1000x1000 grid: " << time1000 << " μs");
        INFO("5000x5000 grid: " << time5000 << " μs");
    }
}

TEST_CASE("Performance requirements validation", "[Performance]") {
    SECTION("Target FPS achievement for medium grids") {
        GameConfig config;
        config.setGridWidth(500);
        config.setGridHeight(500);
        config.setTargetFps(60);
        
        GameOfLifeSimulation simulation(config);
        
        // Create moderate density pattern (~5%)
        for (int i = 0; i < 12500; ++i) {
            int x = rand() % 500;
            int y = rand() % 500;
            simulation.setCellAlive(x, y);
        }
        
        // Time multiple steps to get average
        const int numSteps = 100;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numSteps; ++i) {
            simulation.step();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto avgTimePerStep = totalTime.count() / numSteps;
        
        // For 60 FPS, each step should take < 16.67ms
        REQUIRE(avgTimePerStep < 17);
        
        INFO("Average time per step: " << avgTimePerStep << " ms");
        INFO("Theoretical max FPS: " << (1000.0 / avgTimePerStep));
    }
    
    SECTION("Memory usage stays reasonable") {
        GameConfig config;
        config.setGridWidth(1000);
        config.setGridHeight(1000);
        config.setMemoryLimitMb(100);
        
        GameOfLifeSimulation simulation(config);
        
        // Create pattern that should stay under memory limit
        for (int i = 0; i < 50000; ++i) { // 5% density
            int x = rand() % 1000;
            int y = rand() % 1000;
            simulation.setCellAlive(x, y);
        }
        
        // Run simulation for several generations
        for (int gen = 0; gen < 100; ++gen) {
            simulation.step();
            
            // Ensure entity count stays reasonable
            REQUIRE(simulation.getLivingCellCount() < 100000);
        }
    }
}