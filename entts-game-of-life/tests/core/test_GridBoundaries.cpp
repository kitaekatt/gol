#include <catch2/catch_test_macros.hpp>
#include "core/GameOfLifeSimulation.h"
#include "core/GameConfig.h"

TEST_CASE("Grid boundary handling - non-wrapping", "[GridBoundaries]") {
    GameConfig config;
    config.setGridWidth(5);
    config.setGridHeight(5);
    config.setWrapEdges(false);
    
    GameOfLifeSimulation simulation(config);
    
    SECTION("Cells outside grid boundaries are considered dead") {
        // Set up pattern at top-left corner
        simulation.setCellAlive(0, 0);
        simulation.setCellAlive(0, 1);
        simulation.setCellAlive(1, 0);
        
        // Cell at (0,0) should only count neighbors within grid
        REQUIRE(simulation.getNeighborCount(0, 0) == 2);
        
        // Cell at (-1, -1) is outside grid, should be considered dead
        REQUIRE(simulation.isCellAlive(-1, -1) == false);
    }
    
    SECTION("Edge cells have fewer neighbors than interior cells") {
        // Fill entire 5x5 grid
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                simulation.setCellAlive(x, y);
            }
        }
        
        // Corner cells have 3 neighbors
        REQUIRE(simulation.getNeighborCount(0, 0) == 3);
        REQUIRE(simulation.getNeighborCount(0, 4) == 3);
        REQUIRE(simulation.getNeighborCount(4, 0) == 3);
        REQUIRE(simulation.getNeighborCount(4, 4) == 3);
        
        // Edge cells have 5 neighbors
        REQUIRE(simulation.getNeighborCount(0, 2) == 5);
        REQUIRE(simulation.getNeighborCount(2, 0) == 5);
        REQUIRE(simulation.getNeighborCount(4, 2) == 5);
        REQUIRE(simulation.getNeighborCount(2, 4) == 5);
        
        // Interior cells have 8 neighbors
        REQUIRE(simulation.getNeighborCount(2, 2) == 8);
    }
    
    SECTION("Patterns at grid edges behave correctly") {
        // Glider hitting top edge
        simulation.setCellAlive(1, 0); // Top edge
        simulation.setCellAlive(2, 1);
        simulation.setCellAlive(0, 2);
        simulation.setCellAlive(1, 2);
        simulation.setCellAlive(2, 2);
        
        simulation.step();
        
        // Glider should be affected by edge boundary
        REQUIRE(simulation.getLivingCellCount() > 0);
        REQUIRE(simulation.getLivingCellCount() <= 5);
    }
}

TEST_CASE("Grid boundary handling - wrapping edges", "[GridBoundaries]") {
    GameConfig config;
    config.setGridWidth(5);
    config.setGridHeight(5);
    config.setWrapEdges(true);
    
    GameOfLifeSimulation simulation(config);
    
    SECTION("Cells wrap around grid edges") {
        // Set cell at top-left corner
        simulation.setCellAlive(0, 0);
        
        // Set neighbors that would be outside grid but wrap around
        simulation.setCellAlive(4, 4); // Bottom-right wraps to be neighbor of (0,0)
        simulation.setCellAlive(4, 0); // Left edge wraps to right
        simulation.setCellAlive(0, 4); // Top edge wraps to bottom
        
        // Cell at (0,0) should count wrapped neighbors
        REQUIRE(simulation.getNeighborCount(0, 0) == 3);
    }
    
    SECTION("All cells in wrapping grid have potential for 8 neighbors") {
        // Fill entire grid except one corner
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                if (!(x == 0 && y == 0)) {
                    simulation.setCellAlive(x, y);
                }
            }
        }
        
        // Corner cell should have 8 neighbors due to wrapping
        REQUIRE(simulation.getNeighborCount(0, 0) == 8);
    }
    
    SECTION("Glider travels across wrapped boundaries") {
        // Set up glider near right edge
        simulation.setCellAlive(3, 1);
        simulation.setCellAlive(4, 2);
        simulation.setCellAlive(2, 3);
        simulation.setCellAlive(3, 3);
        simulation.setCellAlive(4, 3);
        
        // Step several times to let glider cross boundary
        for (int i = 0; i < 10; ++i) {
            simulation.step();
        }
        
        // Glider should still exist somewhere (conservation in wrapping world)
        REQUIRE(simulation.getLivingCellCount() > 0);
    }
}

TEST_CASE("Grid coordinate validation", "[GridBoundaries]") {
    GameConfig config;
    config.setGridWidth(10);
    config.setGridHeight(8);
    config.setWrapEdges(false);
    
    GameOfLifeSimulation simulation(config);
    
    SECTION("Valid coordinates are accepted") {
        REQUIRE_NOTHROW(simulation.setCellAlive(0, 0));
        REQUIRE_NOTHROW(simulation.setCellAlive(9, 7));
        REQUIRE_NOTHROW(simulation.setCellAlive(5, 4));
    }
    
    SECTION("Invalid coordinates are handled gracefully") {
        // These should not crash but may be ignored or wrapped depending on implementation
        REQUIRE_NOTHROW(simulation.setCellAlive(-1, -1));
        REQUIRE_NOTHROW(simulation.setCellAlive(10, 8));
        REQUIRE_NOTHROW(simulation.setCellAlive(15, 15));
        
        // Setting invalid coordinates should not create cells
        simulation.setCellAlive(-1, -1);
        simulation.setCellAlive(10, 8);
        REQUIRE(simulation.getLivingCellCount() == 0);
    }
    
    SECTION("Neighbor counting handles out-of-bounds correctly") {
        // Cell at edge should have correct neighbor count
        simulation.setCellAlive(0, 0);
        simulation.setCellAlive(0, 1);
        simulation.setCellAlive(1, 0);
        
        // Should not count non-existent neighbors
        REQUIRE(simulation.getNeighborCount(0, 0) == 2);
    }
}

TEST_CASE("Large grid boundaries", "[GridBoundaries]") {
    GameConfig config;
    config.setGridWidth(1000);
    config.setGridHeight(1000);
    config.setWrapEdges(false);
    
    GameOfLifeSimulation simulation(config);
    
    SECTION("Large grid coordinates work correctly") {
        simulation.setCellAlive(999, 999); // Bottom-right corner
        simulation.setCellAlive(998, 999);
        simulation.setCellAlive(999, 998);
        
        REQUIRE(simulation.getNeighborCount(999, 999) == 2);
        REQUIRE(simulation.isCellAlive(999, 999) == true);
    }
    
    SECTION("Performance with sparse large grid") {
        // Add a few cells in large grid - should be efficient
        simulation.setCellAlive(500, 500);
        simulation.setCellAlive(500, 501);
        simulation.setCellAlive(501, 500);
        
        // Should be fast even with large grid
        auto start = std::chrono::high_resolution_clock::now();
        simulation.step();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        REQUIRE(duration.count() < 100); // Should complete in under 100ms
    }
}