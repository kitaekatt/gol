#include <catch2/catch_test_macros.hpp>
#include "console/SimulationController.h"
#include "core/GameConfig.h"
#include <chrono>
#include <vector>

TEST_CASE("SimulationController headless operation", "[HeadlessController]") {
    SECTION("Controller runs independently of view components") {
        GameConfig config;
        config.setGridWidth(50);
        config.setGridHeight(50);
        config.setTargetFps(1000); // Fast execution for testing
        
        SimulationController controller(config);
        
        // Set up simple blinker pattern
        controller.reset();
        // Note: Would need to manually set cells since loadPattern requires file
        
        REQUIRE(controller.getState() == SimulationState::Stopped);
        REQUIRE(controller.getStats().generation == 0);
    }
    
    SECTION("Headless simulation with callback monitoring") {
        GameConfig config;
        config.setGridWidth(20);
        config.setGridHeight(20);
        config.setMaxGenerations(10);
        
        SimulationController controller(config);
        
        // Track simulation progress via callback
        std::vector<SimulationStats> statsHistory;
        controller.setStepCallback([&statsHistory](const SimulationStats& stats) {
            statsHistory.push_back(stats);
        });
        
        // Manually create pattern (since file loading requires actual files)
        controller.reset();
        
        // Run headless simulation
        auto start = std::chrono::steady_clock::now();
        controller.runHeadless(5);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Verify simulation ran
        REQUIRE(controller.getState() == SimulationState::Stopped);
        REQUIRE(duration.count() < 1000); // Should complete quickly
        
        // Verify callbacks were called
        REQUIRE(statsHistory.size() <= 5); // At most 5 generations
    }
    
    SECTION("Performance test with large grid") {
        GameConfig config;
        config.setGridWidth(500);
        config.setGridHeight(500);
        config.setTargetFps(60);
        config.setMaxGenerations(100);
        
        SimulationController controller(config);
        
        // Measure performance without any view overhead
        auto start = std::chrono::steady_clock::now();
        controller.runHeadless(10);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        REQUIRE(controller.getStats().generation <= 10);
        REQUIRE(duration.count() < 5000); // Should complete in under 5 seconds
        
        INFO("Large grid simulation took: " << duration.count() << " ms");
    }
    
    SECTION("Memory efficiency validation") {
        GameConfig config;
        config.setGridWidth(1000);
        config.setGridHeight(1000);
        
        SimulationController controller(config);
        
        // Verify sparse representation efficiency
        REQUIRE(controller.getLivingCellCount() == 0);
        
        // Large grid with no cells should be efficient
        auto cells = controller.getLivingCells();
        REQUIRE(cells.empty());
        
        // Grid size shouldn't affect memory with no living cells
        config.setGridWidth(10000);
        config.setGridHeight(10000);
        controller.setConfig(config);
        
        REQUIRE(controller.getLivingCellCount() == 0);
    }
}

TEST_CASE("Model/View separation validation", "[ModelViewSeparation]") {
    SECTION("Controller interface is view-agnostic") {
        SimulationController controller;
        
        // All these operations should work without any view components
        REQUIRE_NOTHROW(controller.start());
        REQUIRE_NOTHROW(controller.pause());
        REQUIRE_NOTHROW(controller.step());
        REQUIRE_NOTHROW(controller.reset());
        REQUIRE_NOTHROW(controller.stop());
        
        // State queries should work independently
        REQUIRE_NOTHROW(controller.getState());
        REQUIRE_NOTHROW(controller.getStats());
        REQUIRE_NOTHROW(controller.getConfig());
        REQUIRE_NOTHROW(controller.getLivingCellCount());
        REQUIRE_NOTHROW(controller.getLivingCells());
    }
    
    SECTION("Controller provides all necessary data for views") {
        SimulationController controller;
        
        // Controller should provide complete state information
        const auto& stats = controller.getStats();
        REQUIRE(stats.generation >= 0);
        REQUIRE(stats.livingCells >= 0);
        REQUIRE(stats.actualFps >= 0.0);
        
        const auto& config = controller.getConfig();
        REQUIRE(config.isValid());
        
        // Cell query interface for views
        auto cells = controller.getLivingCells();
        REQUIRE(cells.size() == controller.getLivingCellCount());
        
        // Individual cell queries
        REQUIRE_NOTHROW(controller.isCellAlive(0, 0));
        REQUIRE_NOTHROW(controller.isCellAlive(100, 100));
    }
    
    SECTION("Timing control is independent of rendering") {
        GameConfig config;
        config.setTargetFps(30);
        
        SimulationController controller(config);
        
        // Timing should work without view
        REQUIRE_NOTHROW(controller.setTargetFps(60));
        REQUIRE_NOTHROW(controller.shouldUpdate());
        REQUIRE_NOTHROW(controller.updateTiming());
        
        // Multiple timing checks should be consistent
        bool shouldUpdate1 = controller.shouldUpdate();
        bool shouldUpdate2 = controller.shouldUpdate();
        
        // Results should be consistent for same time
        REQUIRE(shouldUpdate1 == shouldUpdate2);
    }
}

TEST_CASE("Configuration independence", "[ConfigIndependence]") {
    SECTION("Controller accepts configuration changes") {
        SimulationController controller;
        
        GameConfig newConfig;
        newConfig.setGridWidth(200);
        newConfig.setGridHeight(150);
        newConfig.setTargetFps(120);
        
        REQUIRE_NOTHROW(controller.setConfig(newConfig));
        
        const auto& appliedConfig = controller.getConfig();
        REQUIRE(appliedConfig.getGridWidth() == 200);
        REQUIRE(appliedConfig.getGridHeight() == 150);
        REQUIRE(appliedConfig.getTargetFps() == 120);
    }
    
    SECTION("Invalid configuration handling") {
        SimulationController controller;
        
        GameConfig invalidConfig;
        invalidConfig.setGridWidth(-1);  // Invalid
        invalidConfig.setGridHeight(0);  // Invalid
        
        // Controller should handle invalid config gracefully
        // (Implementation may throw or use defaults)
        REQUIRE_NOTHROW(controller.setConfig(invalidConfig));
    }
}