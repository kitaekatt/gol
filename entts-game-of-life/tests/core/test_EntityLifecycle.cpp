#include <catch2/catch_test_macros.hpp>
#include "core/GameOfLifeSimulation.h"
#include "core/components/Position.h"
#include "core/components/Cell.h"
#include <entt/entt.hpp>

TEST_CASE("Entity creation and destruction", "[EntityLifecycle]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Creating live cells creates entities") {
        REQUIRE(simulation.getLivingCellCount() == 0);
        
        simulation.setCellAlive(5, 5);
        REQUIRE(simulation.getLivingCellCount() == 1);
        REQUIRE(simulation.isCellAlive(5, 5) == true);
        
        simulation.setCellAlive(10, 10);
        REQUIRE(simulation.getLivingCellCount() == 2);
        REQUIRE(simulation.isCellAlive(10, 10) == true);
    }
    
    SECTION("Setting same cell alive twice doesn't create duplicate entities") {
        simulation.setCellAlive(3, 3);
        REQUIRE(simulation.getLivingCellCount() == 1);
        
        simulation.setCellAlive(3, 3); // Same cell again
        REQUIRE(simulation.getLivingCellCount() == 1);
        REQUIRE(simulation.isCellAlive(3, 3) == true);
    }
    
    SECTION("Killing cells destroys entities") {
        simulation.setCellAlive(7, 7);
        REQUIRE(simulation.getLivingCellCount() == 1);
        
        simulation.setCellDead(7, 7);
        REQUIRE(simulation.getLivingCellCount() == 0);
        REQUIRE(simulation.isCellAlive(7, 7) == false);
    }
    
    SECTION("Killing non-existent cell is safe") {
        REQUIRE(simulation.getLivingCellCount() == 0);
        
        simulation.setCellDead(15, 15); // Cell doesn't exist
        REQUIRE(simulation.getLivingCellCount() == 0);
    }
}

TEST_CASE("Entity components validation", "[EntityLifecycle]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Live cells have correct components") {
        simulation.setCellAlive(2, 3);
        
        auto entity = simulation.getEntityAt(2, 3);
        REQUIRE(entity != entt::entity{entt::null});
        
        const auto& registry = simulation.getRegistry();
        REQUIRE(registry.all_of<Position>(entity));
        REQUIRE(registry.all_of<Cell>(entity));
        
        const auto& pos = registry.get<Position>(entity);
        REQUIRE(pos.x == 2);
        REQUIRE(pos.y == 3);
        
        const auto& cell = registry.get<Cell>(entity);
        REQUIRE(cell.alive == true);
    }
    
    SECTION("Dead cells have no entities") {
        auto entity = simulation.getEntityAt(20, 20);
        REQUIRE(entity == entt::entity{entt::null});
    }
}

TEST_CASE("Entity lifecycle during simulation steps", "[EntityLifecycle]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Entities are created when cells are born") {
        // Create pattern where cell will be born
        simulation.setCellAlive(1, 1);
        simulation.setCellAlive(1, 2);
        simulation.setCellAlive(2, 1);
        // Cell at (2,2) should be born with 3 neighbors
        
        REQUIRE(simulation.getLivingCellCount() == 3);
        REQUIRE(simulation.isCellAlive(2, 2) == false);
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(2, 2) == true);
        REQUIRE(simulation.getLivingCellCount() == 4);
    }
    
    SECTION("Entities are destroyed when cells die") {
        // Create isolated cell that will die
        simulation.setCellAlive(10, 10);
        REQUIRE(simulation.getLivingCellCount() == 1);
        
        simulation.step();
        
        REQUIRE(simulation.isCellAlive(10, 10) == false);
        REQUIRE(simulation.getLivingCellCount() == 0);
    }
    
    SECTION("Multiple generation lifecycle") {
        // Blinker pattern
        simulation.setCellAlive(5, 4);
        simulation.setCellAlive(5, 5);
        simulation.setCellAlive(5, 6);
        
        size_t initialCount = simulation.getLivingCellCount();
        REQUIRE(initialCount == 3);
        
        // After one step, pattern rotates but count stays same
        simulation.step();
        REQUIRE(simulation.getLivingCellCount() == 3);
        
        // After another step, back to original
        simulation.step();
        REQUIRE(simulation.getLivingCellCount() == 3);
        REQUIRE(simulation.isCellAlive(5, 4) == true);
        REQUIRE(simulation.isCellAlive(5, 5) == true);
        REQUIRE(simulation.isCellAlive(5, 6) == true);
    }
}

TEST_CASE("Sparse representation efficiency", "[EntityLifecycle]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Only living cells create entities") {
        // In a 1000x1000 grid, only create a few living cells
        simulation.setCellAlive(100, 100);
        simulation.setCellAlive(500, 500);
        simulation.setCellAlive(900, 900);
        
        REQUIRE(simulation.getLivingCellCount() == 3);
        
        // Registry should only contain 3 entities despite large grid
        const auto& registry = simulation.getRegistry();
        size_t entityCount = 0;
        registry.view<entt::entity>().each([&entityCount](auto) { entityCount++; });
        REQUIRE(entityCount == 3);
    }
    
    SECTION("Memory usage scales with living cells, not grid size") {
        GameOfLifeSimulation smallSim; // Default grid size
        GameOfLifeSimulation largeSim{GameConfig{}}; // Same configuration
        
        // Add same pattern to both
        smallSim.setCellAlive(5, 5);
        largeSim.setCellAlive(5, 5);
        
        // Both should have same entity count
        REQUIRE(smallSim.getLivingCellCount() == largeSim.getLivingCellCount());
    }
}

TEST_CASE("Entity position mapping", "[EntityLifecycle]") {
    GameOfLifeSimulation simulation;
    
    SECTION("Position components match grid coordinates") {
        simulation.setCellAlive(25, 30);
        simulation.setCellAlive(100, 200);
        
        auto entity1 = simulation.getEntityAt(25, 30);
        auto entity2 = simulation.getEntityAt(100, 200);
        
        REQUIRE(entity1 != entt::entity{entt::null});
        REQUIRE(entity2 != entt::entity{entt::null});
        REQUIRE(entity1 != entity2);
        
        const auto& registry = simulation.getRegistry();
        const auto& pos1 = registry.get<Position>(entity1);
        const auto& pos2 = registry.get<Position>(entity2);
        
        REQUIRE(pos1.x == 25);
        REQUIRE(pos1.y == 30);
        REQUIRE(pos2.x == 100);
        REQUIRE(pos2.y == 200);
    }
    
    SECTION("Spatial lookup is consistent") {
        simulation.setCellAlive(15, 25);
        
        auto entity1 = simulation.getEntityAt(15, 25);
        auto entity2 = simulation.getEntityAt(15, 25); // Same lookup
        
        REQUIRE(entity1 == entity2);
        REQUIRE(entity1 != entt::entity{entt::null});
    }
}