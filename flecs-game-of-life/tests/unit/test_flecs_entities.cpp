#include <catch2/catch_test_macros.hpp>
#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/components.h>
#include <flecs_gol/game_config.h>

using namespace flecs_gol;

TEST_CASE("FLECS Entity Creation and Destruction", "[flecs][entities]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Create single cell entity") {
        auto entity = simulation.createCell(5, 10);
        
        REQUIRE(entity.is_alive());
        REQUIRE(simulation.getCellCount() == 1);
        REQUIRE(simulation.isCellAlive(5, 10));
        
        // Verify entity has required components
        REQUIRE(entity.has<Position>());
        REQUIRE(entity.has<Cell>());
        
        const Position& pos = entity.get<Position>();
        REQUIRE(pos.x == 5);
        REQUIRE(pos.y == 10);
    }
    
    SECTION("Create multiple cell entities") {
        auto cell1 = simulation.createCell(0, 0);
        auto cell2 = simulation.createCell(1, 1);
        auto cell3 = simulation.createCell(-5, 3);
        
        REQUIRE(simulation.getCellCount() == 3);
        REQUIRE(cell1.is_alive());
        REQUIRE(cell2.is_alive());
        REQUIRE(cell3.is_alive());
        REQUIRE(cell1 != cell2);
        REQUIRE(cell2 != cell3);
        REQUIRE(cell1 != cell3);
    }
    
    SECTION("Destroy cell entity") {
        auto entity = simulation.createCell(2, 3);
        REQUIRE(simulation.getCellCount() == 1);
        REQUIRE(simulation.isCellAlive(2, 3));
        
        simulation.destroyCell(2, 3);
        
        REQUIRE(simulation.getCellCount() == 0);
        REQUIRE_FALSE(simulation.isCellAlive(2, 3));
        REQUIRE_FALSE(entity.is_alive());
    }
    
    SECTION("Cannot create duplicate cells at same position") {
        auto cell1 = simulation.createCell(7, 8);
        auto cell2 = simulation.createCell(7, 8);
        
        REQUIRE(simulation.getCellCount() == 1);
        REQUIRE(cell1.is_alive());
        // Second creation should return same entity or invalid entity
        REQUIRE((cell2 == cell1 || !cell2.is_alive()));
    }
}

TEST_CASE("FLECS Component Management", "[flecs][components]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Position component data integrity") {
        auto entity = simulation.createCell(-100, 250);
        
        const Position& pos = entity.get<Position>();
        
        REQUIRE(pos.x == -100);
        REQUIRE(pos.y == 250);
        
        // Position should be immutable after creation
        auto sameEntity = simulation.getCellAt(-100, 250);
        const Position& samePos = sameEntity.get<Position>();
        REQUIRE(samePos.x == -100);
        REQUIRE(samePos.y == 250);
    }
    
    SECTION("Cell component initialization") {
        auto entity = simulation.createCell(0, 0);
        
        const Cell& cell = entity.get<Cell>();
        
        REQUIRE(cell.neighborCount == 0);
        REQUIRE_FALSE(cell.willLive);
    }
    
    SECTION("Component updates during simulation") {
        // Create a pattern where neighbor counts will change
        auto center = simulation.createCell(0, 0);
        auto neighbor = simulation.createCell(1, 0);
        
        // Before step, counts should be initial
        const Cell& centerCell = center.get<Cell>();
        REQUIRE(centerCell.neighborCount == 0);
        
        // After neighbor counting phase
        simulation.updateNeighborCounts();
        
        const Cell& updatedCenterCell = center.get<Cell>();
        REQUIRE(updatedCenterCell.neighborCount == 1);
    }
}

TEST_CASE("FLECS Query Operations", "[flecs][queries]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Query all live cells") {
        simulation.createCell(1, 1);
        simulation.createCell(2, 2);
        simulation.createCell(3, 3);
        
        auto cells = simulation.getAllCells();
        REQUIRE(cells.size() == 3);
        
        // Verify all cells have required components
        for (const auto& entity : cells) {
            REQUIRE(entity.has<Position>());
            REQUIRE(entity.has<Cell>());
        }
    }
    
    SECTION("Query cells in region") {
        // Create cells inside and outside region
        simulation.createCell(0, 0);   // Inside
        simulation.createCell(5, 5);   // Inside
        simulation.createCell(15, 15); // Outside
        simulation.createCell(-10, 2); // Outside
        
        auto cellsInRegion = simulation.getCellsInRegion(-1, 10, -1, 10);
        REQUIRE(cellsInRegion.size() == 2);
        
        // Verify the correct cells are returned
        bool foundOrigin = false, foundFiveFive = false;
        for (const auto& entity : cellsInRegion) {
            const Position& pos = entity.get<Position>();
            if (pos.x == 0 && pos.y == 0) foundOrigin = true;
            if (pos.x == 5 && pos.y == 5) foundFiveFive = true;
        }
        REQUIRE(foundOrigin);
        REQUIRE(foundFiveFive);
    }
    
    SECTION("Query cells by neighbor count") {
        // Create pattern with varying neighbor counts
        simulation.createCell(0, 0);   // Will have 2 neighbors
        simulation.createCell(1, 0);   // Will have 1 neighbor
        simulation.createCell(0, 1);   // Will have 1 neighbor  
        simulation.createCell(10, 10); // Will have 0 neighbors (isolated)
        
        simulation.updateNeighborCounts();
        
        auto isolatedCells = simulation.getCellsWithNeighborCount(0);
        auto singleNeighborCells = simulation.getCellsWithNeighborCount(1);
        auto twoNeighborCells = simulation.getCellsWithNeighborCount(2);
        
        REQUIRE(isolatedCells.size() == 1);
        REQUIRE(singleNeighborCells.size() == 2);
        REQUIRE(twoNeighborCells.size() == 1);
    }
}

TEST_CASE("FLECS Performance Characteristics", "[flecs][performance]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Entity creation scales linearly") {
        const int numCells = 1000;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numCells; ++i) {
            simulation.createCell(i % 100, i / 100);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        REQUIRE(simulation.getCellCount() == numCells);
        
        // Should be reasonably fast (less than 10ms for 1000 entities)
        REQUIRE(duration.count() < 10000);
    }
    
    SECTION("Query performance scales with result size") {
        // Create many entities
        for (int x = 0; x < 50; ++x) {
            for (int y = 0; y < 50; ++y) {
                simulation.createCell(x, y);
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto allCells = simulation.getAllCells();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        REQUIRE(allCells.size() == 2500);
        
        // Query should be fast even for many entities
        REQUIRE(duration.count() < 1000); // Less than 1ms
    }
    
    SECTION("Memory usage grows linearly with entity count") {
        size_t initialMemory = simulation.getMemoryUsage();
        
        // Add entities and measure memory growth
        for (int i = 0; i < 100; ++i) {
            simulation.createCell(i, 0);
        }
        
        size_t finalMemory = simulation.getMemoryUsage();
        size_t memoryPerEntity = (finalMemory - initialMemory) / 100;
        
        // Each entity should use reasonable amount of memory (< 1KB)
        REQUIRE(memoryPerEntity < 1024);
        REQUIRE(memoryPerEntity > 0);
    }
}

TEST_CASE("FLECS World State Management", "[flecs][state]") {
    GameConfig config;
    GameOfLifeSimulation simulation(config);
    
    SECTION("Clear all entities") {
        simulation.createCell(1, 1);
        simulation.createCell(2, 2);
        simulation.createCell(3, 3);
        REQUIRE(simulation.getCellCount() == 3);
        
        simulation.clear();
        
        REQUIRE(simulation.getCellCount() == 0);
        REQUIRE_FALSE(simulation.isCellAlive(1, 1));
        REQUIRE_FALSE(simulation.isCellAlive(2, 2));
        REQUIRE_FALSE(simulation.isCellAlive(3, 3));
    }
    
    SECTION("Generation counter increments") {
        REQUIRE(simulation.getGeneration() == 0);
        
        simulation.createCell(0, 0);
        simulation.step();
        
        REQUIRE(simulation.getGeneration() == 1);
        
        simulation.step();
        
        REQUIRE(simulation.getGeneration() == 2);
    }
    
    SECTION("Reset simulation state") {
        simulation.createCell(5, 5);
        simulation.step();
        simulation.step();
        
        REQUIRE(simulation.getGeneration() == 2);
        REQUIRE(simulation.getCellCount() == 0); // Cell died from underpopulation
        
        simulation.reset();
        
        REQUIRE(simulation.getGeneration() == 0);
        REQUIRE(simulation.getCellCount() == 0);
    }
}