#include <catch2/catch_test_macros.hpp>
#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/components.h>
#include <flecs_gol/game_config.h>

using namespace flecs_gol;

TEST_CASE("Conway's Game of Life Rules - Cell Death", "[rules]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Cell dies with 0 neighbors (underpopulation)") {
        // Create a single isolated cell
        auto cell = simulation.createCell(0, 0);
        REQUIRE(simulation.getCellCount() == 1);
        
        simulation.step();
        
        // Cell should die due to underpopulation
        REQUIRE(simulation.getCellCount() == 0);
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
    }
    
    SECTION("Cell dies with 1 neighbor (underpopulation)") {
        // Create two adjacent cells
        simulation.createCell(0, 0);
        simulation.createCell(1, 0);
        REQUIRE(simulation.getCellCount() == 2);
        
        simulation.step();
        
        // Both cells should die due to underpopulation
        REQUIRE(simulation.getCellCount() == 0);
    }
    
    SECTION("Cell dies with 4 neighbors (overpopulation)") {
        // Create a cell surrounded by 4 neighbors
        simulation.createCell(0, 0);  // Center cell
        simulation.createCell(-1, 0); // Left
        simulation.createCell(1, 0);  // Right  
        simulation.createCell(0, -1); // Top
        simulation.createCell(0, 1);  // Bottom
        REQUIRE(simulation.getCellCount() == 5);
        
        simulation.step();
        
        // Center cell should die due to overpopulation
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
        // The 4 neighbors should also die (they each have only 2 neighbors)
        REQUIRE(simulation.getCellCount() == 4);
    }
    
    SECTION("Cell dies with 5+ neighbors (overpopulation)") {
        // Create a cell surrounded by 5 neighbors
        simulation.createCell(0, 0);   // Center
        simulation.createCell(-1, -1);
        simulation.createCell(-1, 0);
        simulation.createCell(-1, 1);
        simulation.createCell(0, -1);
        simulation.createCell(0, 1);
        REQUIRE(simulation.getCellCount() == 6);
        
        simulation.step();
        
        // Center cell should die due to overpopulation
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
    }
}

TEST_CASE("Conway's Game of Life Rules - Cell Survival", "[rules]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Cell survives with 2 neighbors") {
        // Create an L-shape (stable with 2 neighbors each)
        simulation.createCell(0, 0);
        simulation.createCell(1, 0);
        simulation.createCell(0, 1);
        REQUIRE(simulation.getCellCount() == 3);
        
        simulation.step();
        
        // All cells should survive (each has exactly 2 neighbors)
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE(simulation.isCellAlive(0, 0));
        REQUIRE(simulation.isCellAlive(1, 0));
        REQUIRE(simulation.isCellAlive(0, 1));
    }
    
    SECTION("Cell survives with 3 neighbors") {
        // Create a small cluster where center has 3 neighbors
        simulation.createCell(0, 0);   // Center
        simulation.createCell(-1, 0);  // Left
        simulation.createCell(1, 0);   // Right
        simulation.createCell(0, 1);   // Bottom
        REQUIRE(simulation.getCellCount() == 4);
        
        simulation.step();
        
        // Center cell should survive with 3 neighbors
        REQUIRE(simulation.isCellAlive(0, 0));
    }
}

TEST_CASE("Conway's Game of Life Rules - Cell Birth", "[rules]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Dead cell becomes alive with exactly 3 neighbors") {
        // Create 3 cells around an empty position
        simulation.createCell(-1, 0);  // Left of origin
        simulation.createCell(1, 0);   // Right of origin  
        simulation.createCell(0, 1);   // Below origin
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
        
        simulation.step();
        
        // A new cell should be born at (0,0)
        REQUIRE(simulation.isCellAlive(0, 0));
        REQUIRE(simulation.getCellCount() > 3);
    }
    
    SECTION("Dead cell stays dead with 2 neighbors") {
        // Create 2 cells around an empty position
        simulation.createCell(-1, 0);
        simulation.createCell(1, 0);
        REQUIRE(simulation.getCellCount() == 2);
        
        simulation.step();
        
        // No new cell should be born at (0,0)
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
    }
    
    SECTION("Dead cell stays dead with 4+ neighbors") {
        // Create 4 cells around an empty position
        simulation.createCell(-1, 0);
        simulation.createCell(1, 0);
        simulation.createCell(0, -1);
        simulation.createCell(0, 1);
        REQUIRE(simulation.getCellCount() == 4);
        
        simulation.step();
        
        // No new cell should be born at (0,0) due to overcrowding
        REQUIRE_FALSE(simulation.isCellAlive(0, 0));
    }
}

TEST_CASE("Conway's Game of Life Rules - Known Patterns", "[rules]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Blinker oscillator (period 2)") {
        // Create horizontal blinker
        simulation.createCell(-1, 0);
        simulation.createCell(0, 0);
        simulation.createCell(1, 0);
        REQUIRE(simulation.getCellCount() == 3);
        
        // After one step, should become vertical
        simulation.step();
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE(simulation.isCellAlive(0, -1));
        REQUIRE(simulation.isCellAlive(0, 0));
        REQUIRE(simulation.isCellAlive(0, 1));
        REQUIRE_FALSE(simulation.isCellAlive(-1, 0));
        REQUIRE_FALSE(simulation.isCellAlive(1, 0));
        
        // After second step, should return to horizontal
        simulation.step();
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE(simulation.isCellAlive(-1, 0));
        REQUIRE(simulation.isCellAlive(0, 0));
        REQUIRE(simulation.isCellAlive(1, 0));
        REQUIRE_FALSE(simulation.isCellAlive(0, -1));
        REQUIRE_FALSE(simulation.isCellAlive(0, 1));
    }
    
    SECTION("Block still life (stable)") {
        // Create 2x2 block
        simulation.createCell(0, 0);
        simulation.createCell(1, 0);
        simulation.createCell(0, 1);
        simulation.createCell(1, 1);
        REQUIRE(simulation.getCellCount() == 4);
        
        // Should remain stable
        simulation.step();
        REQUIRE(simulation.getCellCount() == 4);
        REQUIRE(simulation.isCellAlive(0, 0));
        REQUIRE(simulation.isCellAlive(1, 0));
        REQUIRE(simulation.isCellAlive(0, 1));
        REQUIRE(simulation.isCellAlive(1, 1));
        
        // Should still be stable after multiple steps
        simulation.step();
        REQUIRE(simulation.getCellCount() == 4);
    }
}

TEST_CASE("Neighbor Counting Accuracy", "[rules]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Count neighbors correctly for center cell") {
        // Create center cell with various neighbor configurations
        simulation.createCell(0, 0);
        
        // Add neighbors one by one and verify count
        REQUIRE(simulation.getNeighborCount(0, 0) == 0);
        
        simulation.createCell(-1, -1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 1);
        
        simulation.createCell(-1, 0);
        REQUIRE(simulation.getNeighborCount(0, 0) == 2);
        
        simulation.createCell(-1, 1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 3);
        
        simulation.createCell(0, -1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 4);
        
        simulation.createCell(0, 1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 5);
        
        simulation.createCell(1, -1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 6);
        
        simulation.createCell(1, 0);
        REQUIRE(simulation.getNeighborCount(0, 0) == 7);
        
        simulation.createCell(1, 1);
        REQUIRE(simulation.getNeighborCount(0, 0) == 8);
    }
    
    SECTION("Neighbor count for empty cell") {
        // Test neighbor counting for a position with no cell
        simulation.createCell(-1, 0);
        simulation.createCell(1, 0);
        simulation.createCell(0, 1);
        
        // Position (0,0) has no cell but should count 3 neighbors
        REQUIRE(simulation.getNeighborCount(0, 0) == 3);
    }
}