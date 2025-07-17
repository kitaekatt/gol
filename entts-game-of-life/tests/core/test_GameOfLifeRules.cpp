#include <catch2/catch_test_macros.hpp>
#include "core/GameOfLifeSimulation.h"
#include "core/components/Position.h"
#include "core/components/Cell.h"
#include <entt/entt.hpp>

TEST_CASE("Conway's Game of Life Rules", "[GameOfLifeRules]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Rule 1: Live cell with 2-3 neighbors survives") {
        // Create a 3x3 grid with center cell alive and 2 neighbors
        simulation.setCellAlive(1, 1); // Center
        simulation.setCellAlive(0, 1); // Left neighbor
        simulation.setCellAlive(2, 1); // Right neighbor
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == true);
    }
    
    SECTION("Rule 1: Live cell with 3 neighbors survives") {
        // Create center cell with 3 neighbors
        simulation.setCellAlive(1, 1); // Center
        simulation.setCellAlive(0, 1); // Left
        simulation.setCellAlive(2, 1); // Right  
        simulation.setCellAlive(1, 0); // Top
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == true);
    }
    
    SECTION("Rule 2: Live cell with <2 neighbors dies (underpopulation)") {
        // Create center cell with only 1 neighbor
        simulation.setCellAlive(1, 1); // Center
        simulation.setCellAlive(0, 1); // Single neighbor
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == false);
    }
    
    SECTION("Rule 2: Live cell with no neighbors dies") {
        // Create isolated cell
        simulation.setCellAlive(1, 1); // Center, no neighbors
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == false);
    }
    
    SECTION("Rule 3: Live cell with >3 neighbors dies (overpopulation)") {
        // Create center cell with 4 neighbors
        simulation.setCellAlive(1, 1); // Center
        simulation.setCellAlive(0, 1); // Left
        simulation.setCellAlive(2, 1); // Right
        simulation.setCellAlive(1, 0); // Top
        simulation.setCellAlive(1, 2); // Bottom
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == false);
    }
    
    SECTION("Rule 4: Dead cell with exactly 3 neighbors becomes alive (reproduction)") {
        // Create dead center cell with 3 live neighbors
        simulation.setCellAlive(0, 1); // Left
        simulation.setCellAlive(2, 1); // Right
        simulation.setCellAlive(1, 0); // Top
        // Center (1,1) is dead
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == true);
    }
    
    SECTION("Rule 4: Dead cell with <3 neighbors stays dead") {
        // Create dead center cell with 2 neighbors
        simulation.setCellAlive(0, 1); // Left
        simulation.setCellAlive(2, 1); // Right
        // Center (1,1) is dead
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == false);
    }
    
    SECTION("Rule 4: Dead cell with >3 neighbors stays dead") {
        // Create dead center cell with 4 neighbors
        simulation.setCellAlive(0, 1); // Left
        simulation.setCellAlive(2, 1); // Right
        simulation.setCellAlive(1, 0); // Top
        simulation.setCellAlive(1, 2); // Bottom
        // Center (1,1) is dead
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(1, 1) == false);
    }
}

TEST_CASE("Classic Game of Life Patterns", "[GameOfLifePatterns]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Blinker oscillator (period 2)") {
        // Vertical blinker
        simulation.setCellAlive(1, 0);
        simulation.setCellAlive(1, 1);
        simulation.setCellAlive(1, 2);
        
        // After 1 step, should be horizontal
        simulation.step();
        REQUIRE(simulation.isCellAlive(0, 1) == true);
        REQUIRE(simulation.isCellAlive(1, 1) == true);
        REQUIRE(simulation.isCellAlive(2, 1) == true);
        REQUIRE(simulation.isCellAlive(1, 0) == false);
        REQUIRE(simulation.isCellAlive(1, 2) == false);
        
        // After another step, should be vertical again
        simulation.step();
        REQUIRE(simulation.isCellAlive(1, 0) == true);
        REQUIRE(simulation.isCellAlive(1, 1) == true);
        REQUIRE(simulation.isCellAlive(1, 2) == true);
        REQUIRE(simulation.isCellAlive(0, 1) == false);
        REQUIRE(simulation.isCellAlive(2, 1) == false);
    }
    
    SECTION("Block still life (stable)") {
        // 2x2 block
        simulation.setCellAlive(1, 1);
        simulation.setCellAlive(1, 2);
        simulation.setCellAlive(2, 1);
        simulation.setCellAlive(2, 2);
        
        // Should remain stable
        simulation.step();
        REQUIRE(simulation.isCellAlive(1, 1) == true);
        REQUIRE(simulation.isCellAlive(1, 2) == true);
        REQUIRE(simulation.isCellAlive(2, 1) == true);
        REQUIRE(simulation.isCellAlive(2, 2) == true);
        REQUIRE(simulation.getLivingCellCount() == 4);
    }
}

TEST_CASE("Neighbor counting accuracy", "[NeighborCounting]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Cell with all 8 neighbors") {
        // Create 3x3 filled pattern
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                simulation.setCellAlive(x, y);
            }
        }
        
        // Center cell should have 8 neighbors
        REQUIRE(simulation.getNeighborCount(1, 1) == 8);
        
        // Corner cells should have 3 neighbors each
        REQUIRE(simulation.getNeighborCount(0, 0) == 3);
        REQUIRE(simulation.getNeighborCount(0, 2) == 3);
        REQUIRE(simulation.getNeighborCount(2, 0) == 3);
        REQUIRE(simulation.getNeighborCount(2, 2) == 3);
        
        // Edge cells should have 5 neighbors each
        REQUIRE(simulation.getNeighborCount(0, 1) == 5);
        REQUIRE(simulation.getNeighborCount(1, 0) == 5);
        REQUIRE(simulation.getNeighborCount(1, 2) == 5);
        REQUIRE(simulation.getNeighborCount(2, 1) == 5);
    }
    
    SECTION("Diagonal neighbors are counted") {
        simulation.setCellAlive(1, 1); // Center
        simulation.setCellAlive(0, 0); // Top-left diagonal
        simulation.setCellAlive(2, 2); // Bottom-right diagonal
        
        REQUIRE(simulation.getNeighborCount(1, 1) == 2);
        REQUIRE(simulation.getNeighborCount(0, 0) == 1);
        REQUIRE(simulation.getNeighborCount(2, 2) == 1);
    }
}