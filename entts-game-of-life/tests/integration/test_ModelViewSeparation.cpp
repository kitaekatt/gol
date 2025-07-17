#include <catch2/catch_test_macros.hpp>
#include "console/SimulationController.h"
#include "console/ConsoleRenderer.h"
#include "console/ConsoleInput.h"
#include "core/GameConfig.h"
#include <memory>
#include <chrono>

TEST_CASE("Model-View separation validation", "[ModelViewSeparation]") {
    SECTION("Controller operates independently of view components") {
        // Create controller without any view components
        GameConfig config;
        config.setGridWidth(100);
        config.setGridHeight(100);
        config.setTargetFps(1000);
        
        auto controller = std::make_unique<SimulationController>(config);
        
        // All controller operations should work without view
        REQUIRE_NOTHROW(controller->start());
        REQUIRE_NOTHROW(controller->step());
        REQUIRE_NOTHROW(controller->pause());
        REQUIRE_NOTHROW(controller->reset());
        REQUIRE_NOTHROW(controller->stop());
        
        // State queries should work
        REQUIRE(controller->getState() == SimulationState::Stopped);
        REQUIRE(controller->getStats().generation >= 0);
        REQUIRE(controller->getLivingCellCount() >= 0);
        
        // Configuration changes should work
        GameConfig newConfig;
        newConfig.setGridWidth(200);
        newConfig.setGridHeight(150);
        REQUIRE_NOTHROW(controller->setConfig(newConfig));
        
        REQUIRE(controller->getConfig().getGridWidth() == 200);
        REQUIRE(controller->getConfig().getGridHeight() == 150);
    }
    
    SECTION("View components depend only on controller interface") {
        GameConfig config;
        SimulationController controller(config);
        
        // Renderer should only need controller reference
        RenderConfig renderConfig;
        ConsoleRenderer renderer(renderConfig);
        
        // Renderer should be able to query all necessary data from controller
        REQUIRE_NOTHROW(renderer.render(controller));
        
        // Input should be independent of both controller and renderer
        ConsoleInput input;
        REQUIRE_NOTHROW(input.pollInput());
        REQUIRE_NOTHROW(input.getState());
    }
    
    SECTION("Multiple view instances can observe same controller") {
        SimulationController controller;
        
        // Create multiple renderer instances
        RenderConfig config1;
        config1.aliveChar = '#';
        config1.viewportWidth = 40;
        
        RenderConfig config2;
        config2.aliveChar = '*';
        config2.viewportWidth = 80;
        
        ConsoleRenderer renderer1(config1);
        ConsoleRenderer renderer2(config2);
        
        // Both should be able to render same controller state
        REQUIRE_NOTHROW(renderer1.render(controller));
        REQUIRE_NOTHROW(renderer2.render(controller));
        
        // Controller state should be consistent for both
        auto cells1 = controller.getLivingCells();
        auto cells2 = controller.getLivingCells();
        REQUIRE(cells1.size() == cells2.size());
    }
    
    SECTION("Controller provides complete interface for Unity integration") {
        SimulationController controller;
        
        // All methods needed for Unity should be available
        
        // Simulation control
        REQUIRE_NOTHROW(controller.start());
        REQUIRE_NOTHROW(controller.pause());
        REQUIRE_NOTHROW(controller.step());
        REQUIRE_NOTHROW(controller.reset());
        REQUIRE_NOTHROW(controller.stop());
        
        // State queries
        auto state = controller.getState();
        auto stats = controller.getStats();
        auto config = controller.getConfig();
        auto cellCount = controller.getLivingCellCount();
        auto cells = controller.getLivingCells();
        
        REQUIRE(static_cast<int>(state) >= 0);
        REQUIRE(stats.generation >= 0);
        REQUIRE(config.isValid());
        REQUIRE(cellCount >= 0);
        REQUIRE(cells.size() == cellCount);
        
        // Individual cell queries
        bool alive = controller.isCellAlive(0, 0);
        REQUIRE((alive == true || alive == false)); // Valid boolean
        
        // Configuration updates
        GameConfig newConfig;
        REQUIRE_NOTHROW(controller.setConfig(newConfig));
        
        // Headless execution with callbacks
        bool callbackCalled = false;
        controller.setStepCallback([&callbackCalled](const SimulationStats&) {
            callbackCalled = true;
        });
        
        controller.runHeadless(1);
        REQUIRE(callbackCalled == true);
    }
}

TEST_CASE("Interface contracts", "[InterfaceContracts]") {
    SECTION("Controller state transitions are predictable") {
        SimulationController controller;
        
        // Initial state
        REQUIRE(controller.getState() == SimulationState::Stopped);
        
        // Start -> Running
        controller.start();
        REQUIRE(controller.getState() == SimulationState::Running);
        
        // Running -> Paused
        controller.pause();
        REQUIRE(controller.getState() == SimulationState::Paused);
        
        // Paused -> Running
        controller.start();
        REQUIRE(controller.getState() == SimulationState::Running);
        
        // Any -> Stopped
        controller.stop();
        REQUIRE(controller.getState() == SimulationState::Stopped);
    }
    
    SECTION("Statistics are always consistent") {
        SimulationController controller;
        
        auto stats = controller.getStats();
        REQUIRE(stats.generation >= 0);
        REQUIRE(stats.livingCells >= 0);
        REQUIRE(stats.actualFps >= 0.0);
        REQUIRE(stats.lastStepTime.count() >= 0);
        
        // After reset, stats should be reset
        controller.reset();
        stats = controller.getStats();
        REQUIRE(stats.generation == 0);
        REQUIRE(stats.livingCells >= 0); // Could be 0 or initial pattern size
    }
    
    SECTION("Cell queries are consistent with living cell count") {
        SimulationController controller;
        
        auto cellCount = controller.getLivingCellCount();
        auto cells = controller.getLivingCells();
        
        REQUIRE(cells.size() == cellCount);
        
        // Each cell in the list should report as alive
        for (const auto& [x, y] : cells) {
            REQUIRE(controller.isCellAlive(x, y) == true);
        }
    }
    
    SECTION("Configuration changes reset simulation state") {
        SimulationController controller;
        
        // Step simulation to change state
        controller.step();
        auto originalGeneration = controller.getStats().generation;
        
        // Change configuration
        GameConfig newConfig;
        newConfig.setGridWidth(50);
        newConfig.setGridHeight(50);
        controller.setConfig(newConfig);
        
        // Should reset to initial state
        REQUIRE(controller.getStats().generation == 0);
        REQUIRE(controller.getState() == SimulationState::Stopped);
        REQUIRE(controller.getConfig().getGridWidth() == 50);
        REQUIRE(controller.getConfig().getGridHeight() == 50);
    }
}

TEST_CASE("Performance isolation", "[PerformanceIsolation]") {
    SECTION("Controller performance unaffected by view complexity") {
        GameConfig config;
        config.setGridWidth(200);
        config.setGridHeight(200);
        config.setTargetFps(1000);
        
        SimulationController controller(config);
        
        // Measure controller performance alone
        auto start = std::chrono::steady_clock::now();
        controller.runHeadless(100);
        auto end = std::chrono::steady_clock::now();
        
        auto headlessTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        REQUIRE(headlessTime.count() < 2000); // Should be fast
        
        INFO("Headless simulation time: " << headlessTime.count() << " ms");
        
        // Reset for next test
        controller.reset();
        
        // Simulate view overhead by querying controller repeatedly
        start = std::chrono::steady_clock::now();
        
        for (int gen = 0; gen < 100; ++gen) {
            controller.step();
            
            // Simulate view queries that might slow down simulation
            auto cells = controller.getLivingCells();
            auto stats = controller.getStats();
            
            // Multiple cell queries (simulating rendering)
            for (int i = 0; i < 100; ++i) {
                controller.isCellAlive(i % config.getGridWidth(), 
                                     i % config.getGridHeight());
            }
        }
        
        end = std::chrono::steady_clock::now();
        auto withViewTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        INFO("With view queries time: " << withViewTime.count() << " ms");
        
        // View queries shouldn't dramatically slow down simulation
        REQUIRE(withViewTime.count() < headlessTime.count() * 5); // At most 5x slower
    }
}