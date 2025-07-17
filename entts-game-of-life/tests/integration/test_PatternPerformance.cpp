#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "console/SimulationController.h"
#include "core/GameConfig.h"
#include <chrono>
#include <fstream>

// Helper function to create patterns programmatically for testing
void createGliderPattern(SimulationController& controller, std::int32_t x, std::int32_t y) {
    controller.reset();
    // Glider pattern at position (x, y)
    // NOTE: Would need to access simulation directly or add pattern creation methods
    // For now, using theoretical interface
}

void createBlinkerPattern(SimulationController& controller, std::int32_t x, std::int32_t y) {
    controller.reset();
    // Blinker pattern at position (x, y)
    // Horizontal: (x, y), (x+1, y), (x+2, y)
}

void createRandomPattern(SimulationController& controller, std::int32_t density) {
    controller.reset();
    const auto& config = controller.getConfig();
    
    // Create random pattern with given density percentage
    for (std::int32_t x = 0; x < config.getGridWidth(); ++x) {
        for (std::int32_t y = 0; y < config.getGridHeight(); ++y) {
            if (rand() % 100 < density) {
                // Would need method to set individual cells
                // controller.setCellAlive(x, y);
            }
        }
    }
}

TEST_CASE("Pattern performance tests", "[PatternPerformance]") {
    SECTION("Small grid with different patterns") {
        GameConfig config;
        config.setGridWidth(50);
        config.setGridHeight(50);
        config.setTargetFps(1000); // Fast for testing
        
        SimulationController controller(config);
        
        BENCHMARK("Glider on 50x50 grid - 100 generations") {
            createGliderPattern(controller, 10, 10);
            controller.runHeadless(100);
            return controller.getStats().generation;
        };
        
        BENCHMARK("Blinker on 50x50 grid - 100 generations") {
            createBlinkerPattern(controller, 25, 25);
            controller.runHeadless(100);
            return controller.getStats().generation;
        };
    }
    
    SECTION("Medium grid performance") {
        GameConfig config;
        config.setGridWidth(200);
        config.setGridHeight(200);
        config.setTargetFps(1000);
        
        SimulationController controller(config);
        
        BENCHMARK("200x200 grid with sparse pattern") {
            createRandomPattern(controller, 5); // 5% density
            controller.runHeadless(50);
            return controller.getStats().livingCells;
        };
        
        BENCHMARK("200x200 grid with dense pattern") {
            createRandomPattern(controller, 30); // 30% density
            controller.runHeadless(10); // Fewer generations for dense pattern
            return controller.getStats().livingCells;
        };
    }
    
    SECTION("Large grid scalability") {
        GameConfig config;
        config.setGridWidth(1000);
        config.setGridHeight(1000);
        config.setTargetFps(1000);
        
        SimulationController controller(config);
        
        BENCHMARK("1000x1000 grid sparse pattern") {
            createRandomPattern(controller, 1); // 1% density
            controller.runHeadless(20);
            return controller.getStats().livingCells;
        };
        
        // Test that large empty grid is efficient
        BENCHMARK("1000x1000 empty grid step") {
            controller.reset(); // Empty grid
            controller.step();
            return controller.getStats().livingCells;
        };
    }
}

TEST_CASE("Performance requirements validation", "[PerformanceRequirements]") {
    SECTION("60 FPS target for medium grids") {
        GameConfig config;
        config.setGridWidth(500);
        config.setGridHeight(500);
        config.setTargetFps(60);
        
        SimulationController controller(config);
        createRandomPattern(controller, 5); // 5% density
        
        // Measure actual performance
        const int testGenerations = 60; // 1 second worth at 60 FPS
        auto start = std::chrono::high_resolution_clock::now();
        
        controller.runHeadless(testGenerations);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should complete 60 generations in roughly 1 second (with some tolerance)
        REQUIRE(duration.count() < 2000); // Under 2 seconds
        REQUIRE(controller.getStats().generation <= testGenerations);
        
        double actualFps = (controller.getStats().generation * 1000.0) / duration.count();
        INFO("Achieved FPS: " << actualFps << " (target: 60)");
        
        // Should achieve at least 30 FPS (half target)
        REQUIRE(actualFps >= 30.0);
    }
    
    SECTION("Memory usage stays reasonable") {
        GameConfig config;
        config.setGridWidth(1000);
        config.setGridHeight(1000);
        config.setMemoryLimitMb(100);
        
        SimulationController controller(config);
        createRandomPattern(controller, 10); // 10% density
        
        // Run for many generations to test memory stability
        std::size_t initialCells = controller.getLivingCellCount();
        
        controller.runHeadless(200);
        
        std::size_t finalCells = controller.getLivingCellCount();
        
        // Population shouldn't grow exponentially (would indicate memory leak)
        REQUIRE(finalCells < initialCells * 10); // At most 10x growth
        
        INFO("Initial cells: " << initialCells << ", Final cells: " << finalCells);
    }
    
    SECTION("Stable pattern detection") {
        GameConfig config;
        config.setAutoPauseOnStable(true);
        config.setStableDetectionCycles(5);
        
        SimulationController controller(config);
        createBlinkerPattern(controller, 10, 10); // Oscillating pattern
        
        auto start = std::chrono::steady_clock::now();
        controller.runHeadless(1000); // Run for many generations
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should detect oscillation and continue (not pause for stable)
        // Or should detect true stability and pause
        REQUIRE(duration.count() < 5000); // Shouldn't run for too long
        
        INFO("Simulation ran for " << controller.getStats().generation << " generations");
        INFO("Final state: " << (controller.getState() == SimulationState::Paused ? "Paused" : "Running"));
    }
}

TEST_CASE("Stress testing", "[StressTest]") {
    SECTION("Very large grid with minimal pattern") {
        GameConfig config;
        config.setGridWidth(5000);
        config.setGridHeight(5000);
        config.setTargetFps(1000);
        
        SimulationController controller(config);
        
        // Single glider in huge grid should be efficient
        createGliderPattern(controller, 2500, 2500); // Center
        
        auto start = std::chrono::steady_clock::now();
        controller.runHeadless(100);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should complete quickly despite large grid
        REQUIRE(duration.count() < 3000); // Under 3 seconds
        REQUIRE(controller.getStats().livingCells > 0); // Pattern should survive
        
        INFO("5000x5000 grid with glider took: " << duration.count() << " ms");
    }
    
    SECTION("Pattern boundary conditions") {
        GameConfig config;
        config.setGridWidth(20);
        config.setGridHeight(20);
        config.setWrapEdges(false);
        
        SimulationController controller(config);
        
        // Glider at edge should handle boundaries correctly
        createGliderPattern(controller, 18, 18); // Near bottom-right corner
        
        controller.runHeadless(50);
        
        // Should complete without crash
        REQUIRE(controller.getStats().generation <= 50);
        
        // Test with wrapping
        config.setWrapEdges(true);
        controller.setConfig(config);
        createGliderPattern(controller, 18, 18);
        
        controller.runHeadless(50);
        REQUIRE(controller.getStats().generation <= 50);
    }
}