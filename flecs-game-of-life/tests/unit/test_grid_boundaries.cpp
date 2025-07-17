#include <catch2/catch_test_macros.hpp>
#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/game_config.h>

using namespace flecs_gol;

TEST_CASE("Grid Boundary Enforcement", "[boundaries]") {
    GameConfig config;
    config.setGridBoundaries(-10, 10, -5, 5);
    config.setWrapEdges(false);
    GameOfLifeSimulation simulation(config);
    
    SECTION("Allow cell creation within boundaries") {
        REQUIRE(simulation.createCell(0, 0).is_alive());
        REQUIRE(simulation.createCell(-10, -5).is_alive());
        REQUIRE(simulation.createCell(10, 5).is_alive());
        REQUIRE(simulation.getCellCount() == 3);
    }
    
    SECTION("Reject cell creation outside boundaries") {
        auto invalidCell1 = simulation.createCell(-11, 0);
        auto invalidCell2 = simulation.createCell(11, 0);
        auto invalidCell3 = simulation.createCell(0, -6);
        auto invalidCell4 = simulation.createCell(0, 6);
        
        REQUIRE_FALSE(invalidCell1.is_alive());
        REQUIRE_FALSE(invalidCell2.is_alive());
        REQUIRE_FALSE(invalidCell3.is_alive());
        REQUIRE_FALSE(invalidCell4.is_alive());
        REQUIRE(simulation.getCellCount() == 0);
    }
    
    SECTION("Cells near boundary edges behave correctly") {
        // Create cells near boundaries
        simulation.createCell(-9, 0);  // Near left edge
        simulation.createCell(9, 0);   // Near right edge
        simulation.createCell(0, -4);  // Near top edge
        simulation.createCell(0, 4);   // Near bottom edge
        
        REQUIRE(simulation.getCellCount() == 4);
        
        // All should survive since they have no neighbors
        simulation.step();
        REQUIRE(simulation.getCellCount() == 0); // Die from underpopulation
    }
}

TEST_CASE("Grid Boundary Edge Wrapping", "[boundaries]") {
    GameConfig config;
    config.setGridBoundaries(-2, 2, -2, 2); // Small 5x5 grid
    config.setWrapEdges(true);
    GameOfLifeSimulation simulation(config);
    
    SECTION("Neighbor counting wraps around edges") {
        // Place cell at edge and neighbors that wrap around
        simulation.createCell(-2, 0);  // Left edge
        simulation.createCell(2, 0);   // Right edge (should be neighbor via wrapping)
        simulation.createCell(-2, -1); // Above left edge cell
        
        // The cell at (-2, 0) should have 2 neighbors:
        // 1. (2, 0) via horizontal wrapping
        // 2. (-2, -1) directly above
        REQUIRE(simulation.getNeighborCount(-2, 0) == 2);
        
        // The cell at (2, 0) should have 1 neighbor:
        // 1. (-2, 0) via horizontal wrapping
        REQUIRE(simulation.getNeighborCount(2, 0) == 1);
    }
    
    SECTION("Birth across wrapped boundaries") {
        // Create pattern that should birth a cell across wrapped boundary
        simulation.createCell(-2, 0);
        simulation.createCell(-1, 0);
        simulation.createCell(2, 0);  // This wraps to be adjacent to (-2, 0)
        
        // Position (-1, 0) has 2 neighbors, position (1, 0) should get 3 neighbors
        // via wrapping and birth a new cell
        simulation.step();
        
        // Check that wrapping affected the simulation correctly
        REQUIRE(simulation.getCellCount() >= 1); // At least some cells should remain/birth
    }
}

TEST_CASE("Grid Boundary Configuration", "[boundaries]") {
    SECTION("Square grid boundaries") {
        GameConfig config;
        config.setGridBoundaries(-50, 50, -50, 50);
        
        REQUIRE(config.getGridWidth() == 101);
        REQUIRE(config.getGridHeight() == 101);
        REQUIRE(config.isPointInBounds(0, 0));
        REQUIRE(config.isPointInBounds(-50, -50));
        REQUIRE(config.isPointInBounds(50, 50));
        REQUIRE_FALSE(config.isPointInBounds(-51, 0));
        REQUIRE_FALSE(config.isPointInBounds(51, 0));
    }
    
    SECTION("Rectangular grid boundaries") {
        GameConfig config;
        config.setGridBoundaries(-100, 200, -25, 75);
        
        REQUIRE(config.getGridWidth() == 301);  // 200 - (-100) + 1
        REQUIRE(config.getGridHeight() == 101); // 75 - (-25) + 1
        REQUIRE(config.isPointInBounds(0, 0));
        REQUIRE(config.isPointInBounds(-100, -25));
        REQUIRE(config.isPointInBounds(200, 75));
        REQUIRE_FALSE(config.isPointInBounds(-101, 0));
        REQUIRE_FALSE(config.isPointInBounds(201, 0));
        REQUIRE_FALSE(config.isPointInBounds(0, -26));
        REQUIRE_FALSE(config.isPointInBounds(0, 76));
    }
    
    SECTION("Single cell grid") {
        GameConfig config;
        config.setGridBoundaries(0, 0, 0, 0);
        
        REQUIRE(config.getGridWidth() == 1);
        REQUIRE(config.getGridHeight() == 1);
        REQUIRE(config.isPointInBounds(0, 0));
        REQUIRE_FALSE(config.isPointInBounds(1, 0));
        REQUIRE_FALSE(config.isPointInBounds(-1, 0));
        REQUIRE_FALSE(config.isPointInBounds(0, 1));
        REQUIRE_FALSE(config.isPointInBounds(0, -1));
    }
}

TEST_CASE("Boundary Interaction with Patterns", "[boundaries]") {
    GameConfig config;
    config.setGridBoundaries(-3, 3, -3, 3);
    config.setWrapEdges(false);
    GameOfLifeSimulation simulation(config);
    
    SECTION("Glider hits boundary and dies") {
        // Create glider pattern near boundary
        simulation.createCell(1, 1);
        simulation.createCell(2, 1);
        simulation.createCell(3, 1);
        simulation.createCell(3, 2);
        simulation.createCell(2, 3);
        
        REQUIRE(simulation.getCellCount() == 5);
        
        // Step several times - glider should eventually hit boundary and partially die
        for (int i = 0; i < 10; ++i) {
            simulation.step();
        }
        
        // Glider should be disrupted by boundary
        REQUIRE(simulation.getCellCount() < 5);
    }
    
    SECTION("Pattern evolution near boundary") {
        // Create simple pattern near edge
        simulation.createCell(-2, 0);
        simulation.createCell(-1, 0);
        simulation.createCell(0, 0);
        
        // This blinker should work normally since it's not at the very edge
        simulation.step();
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE(simulation.isCellAlive(-1, -1));
        REQUIRE(simulation.isCellAlive(-1, 0));
        REQUIRE(simulation.isCellAlive(-1, 1));
    }
}

TEST_CASE("Wrapped Boundary Corner Cases", "[boundaries]") {
    GameConfig config;
    config.setGridBoundaries(0, 2, 0, 2); // 3x3 grid
    config.setWrapEdges(true);
    GameOfLifeSimulation simulation(config);
    
    SECTION("Corner cell wrapping") {
        // Place cell at corner
        simulation.createCell(0, 0);
        
        // Place neighbors that should connect via corner wrapping
        simulation.createCell(2, 2); // Diagonally opposite corner
        simulation.createCell(2, 0); // Same row, opposite side
        simulation.createCell(0, 2); // Same column, opposite side
        
        // Corner cell should see all these as neighbors via wrapping
        int neighbors = simulation.getNeighborCount(0, 0);
        REQUIRE(neighbors == 3);
    }
    
    SECTION("Center cell in wrapped grid") {
        // Fill entire border to test center cell neighbor counting
        simulation.createCell(0, 0);
        simulation.createCell(0, 1);
        simulation.createCell(0, 2);
        simulation.createCell(1, 0);
        simulation.createCell(1, 2);
        simulation.createCell(2, 0);
        simulation.createCell(2, 1);
        simulation.createCell(2, 2);
        
        // Center cell should have 8 neighbors
        REQUIRE(simulation.getNeighborCount(1, 1) == 8);
    }
}