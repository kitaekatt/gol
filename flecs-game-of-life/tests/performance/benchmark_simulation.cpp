#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/game_config.h>
#include <random>

using namespace flecs_gol;

class BenchmarkFixture {
public:
    BenchmarkFixture() {
        config.setGridBoundaries(-500, 500, -500, 500);
        config.setWrapEdges(false);
        config.setEnableProfiling(true);
    }
    
    void createRandomPattern(GameOfLifeSimulation& sim, int numCells, int seed = 42) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> xDist(-400, 400);
        std::uniform_int_distribution<int> yDist(-400, 400);
        
        int created = 0;
        while (created < numCells) {
            int x = xDist(rng);
            int y = yDist(rng);
            if (!sim.isCellAlive(x, y)) {
                sim.createCell(x, y);
                created++;
            }
        }
    }
    
    void createGliderGun(GameOfLifeSimulation& sim, int x = 0, int y = 0) {
        // Gosper's Glider Gun pattern
        std::vector<std::pair<int, int>> pattern = {
            {0, 4}, {0, 5}, {1, 4}, {1, 5},
            {10, 4}, {10, 5}, {10, 6}, {11, 3}, {11, 7},
            {12, 2}, {12, 8}, {13, 2}, {13, 8}, {14, 5},
            {15, 3}, {15, 7}, {16, 4}, {16, 5}, {16, 6}, {17, 5},
            {20, 2}, {20, 3}, {20, 4}, {21, 2}, {21, 3}, {21, 4},
            {22, 1}, {22, 5}, {24, 0}, {24, 1}, {24, 5}, {24, 6},
            {34, 2}, {34, 3}, {35, 2}, {35, 3}
        };
        
        for (const auto& [dx, dy] : pattern) {
            sim.createCell(x + dx, y + dy);
        }
    }

protected:
    GameConfig config;
};

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Entity Creation", "[benchmark][entities]") {
    BENCHMARK("Create 1000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 1000);
        return sim.getCellCount();
    };
    
    BENCHMARK("Create 10000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        return sim.getCellCount();
    };
}

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Simulation Steps", "[benchmark][simulation]") {
    BENCHMARK("Single step - 100 cells") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 100);
        sim.step();
        return sim.getCellCount();
    };
    
    BENCHMARK("Single step - 1000 cells") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 1000);
        sim.step();
        return sim.getCellCount();
    };
    
    BENCHMARK("Single step - 10000 cells") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        sim.step();
        return sim.getCellCount();
    };
    
    BENCHMARK("100 steps - glider gun") {
        GameOfLifeSimulation sim(config);
        createGliderGun(sim);
        for (int i = 0; i < 100; ++i) {
            sim.step();
        }
        return sim.getCellCount();
    };
}

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Neighbor Counting", "[benchmark][neighbors]") {
    BENCHMARK("Neighbor count - dense 50x50 region") {
        GameOfLifeSimulation sim(config);
        // Create dense pattern
        for (int x = -25; x <= 25; ++x) {
            for (int y = -25; y <= 25; ++y) {
                if ((x + y) % 3 == 0) { // Sparse pattern
                    sim.createCell(x, y);
                }
            }
        }
        sim.updateNeighborCounts();
        return sim.getCellCount();
    };
    
    BENCHMARK("Neighbor count - sparse random 10000 cells") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        sim.updateNeighborCounts();
        return sim.getCellCount();
    };
}

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Query Operations", "[benchmark][queries]") {
    BENCHMARK("Query all cells - 1000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 1000);
        auto cells = sim.getAllCells();
        return cells.size();
    };
    
    BENCHMARK("Query all cells - 10000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        auto cells = sim.getAllCells();
        return cells.size();
    };
    
    BENCHMARK("Regional query - 100x100 region from 10000 cells") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        auto cells = sim.getCellsInRegion(-50, 50, -50, 50);
        return cells.size();
    };
}

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Memory Operations", "[benchmark][memory]") {
    BENCHMARK("Memory usage - 1000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 1000);
        return sim.getMemoryUsage();
    };
    
    BENCHMARK("Memory usage - 10000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 10000);
        return sim.getMemoryUsage();
    };
    
    BENCHMARK("Clear and recreate - 5000 entities") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 5000);
        sim.clear();
        createRandomPattern(sim, 5000);
        return sim.getCellCount();
    };
}

TEST_CASE_METHOD(BenchmarkFixture, "Benchmark Complex Patterns", "[benchmark][patterns]") {
    BENCHMARK("Glider evolution - 50 steps") {
        GameOfLifeSimulation sim(config);
        // Create glider
        sim.createCell(1, 0);
        sim.createCell(2, 1);
        sim.createCell(0, 2);
        sim.createCell(1, 2);
        sim.createCell(2, 2);
        
        for (int i = 0; i < 50; ++i) {
            sim.step();
        }
        return sim.getCellCount();
    };
    
    BENCHMARK("Multiple glider guns - 25 steps") {
        GameOfLifeSimulation sim(config);
        // Create multiple glider guns
        createGliderGun(sim, -200, -200);
        createGliderGun(sim, 200, -200);
        createGliderGun(sim, -200, 200);
        createGliderGun(sim, 200, 200);
        
        for (int i = 0; i < 25; ++i) {
            sim.step();
        }
        return sim.getCellCount();
    };
}

// Performance validation tests (not benchmarks, but performance requirements)
TEST_CASE_METHOD(BenchmarkFixture, "Performance Requirements Validation", "[performance][validation]") {
    SECTION("Single step should complete in under 16ms for 1000 cells (60 FPS)") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 1000);
        
        auto start = std::chrono::high_resolution_clock::now();
        sim.step();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        REQUIRE(duration.count() < 16); // 16ms = 60 FPS budget
    }
    
    SECTION("Memory usage should be reasonable for large simulations") {
        GameOfLifeSimulation sim(config);
        createRandomPattern(sim, 100000);
        
        size_t memoryUsage = sim.getMemoryUsage();
        size_t memoryPerCell = memoryUsage / 100000;
        
        // Each cell should use less than 1KB of memory
        REQUIRE(memoryPerCell < 1024);
        REQUIRE(memoryPerCell > 0);
    }
    
    SECTION("Entity creation should scale linearly") {
        auto measureCreationTime = [this](int numCells) {
            GameOfLifeSimulation sim(config);
            auto start = std::chrono::high_resolution_clock::now();
            createRandomPattern(sim, numCells);
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        };
        
        auto time1000 = measureCreationTime(1000);
        auto time2000 = measureCreationTime(2000);
        
        // Time should scale roughly linearly (within 50% tolerance)
        double ratio = static_cast<double>(time2000) / time1000;
        REQUIRE(ratio < 3.0); // Should be roughly 2x, allow up to 3x
        REQUIRE(ratio > 1.0);
    }
}