#include <flecs_gol/game_of_life_simulation.h>
#include <algorithm>
#include <iostream>
#include <set>

namespace flecs_gol {

GameOfLifeSimulation::GameOfLifeSimulation(const GameConfig& config)
    : config_(config)
    , liveCellQuery_(world_.query<Position, Cell>())
    , birthCandidateQuery_(world_.query<Position, BirthCandidate>()) {
    
    // Register components
    world_.component<Position>();
    world_.component<Cell>();
    world_.component<BirthCandidate>();
    world_.component<GridState>();
    world_.component<PerformanceMetrics>();
    
    // Create singleton entities
    gridStateEntity_ = world_.entity().set<GridState>({});
    performanceEntity_ = world_.entity().set<PerformanceMetrics>({});
    
    lastStepTime_ = std::chrono::high_resolution_clock::now();
}

flecs::entity GameOfLifeSimulation::createCell(int32_t x, int32_t y) {
    if (!isValidPosition(x, y)) {
        return flecs::entity(); // Return invalid entity
    }
    
    Position pos(x, y);
    
    // Check if cell already exists at this position
    auto it = spatialIndex_.find(pos);
    if (it != spatialIndex_.end()) {
        return it->second; // Return existing entity
    }
    
    // Create new cell entity
    auto entity = world_.entity()
        .set<Position>(pos)
        .set<Cell>({});
    
    spatialIndex_[pos] = entity;
    
    // Update grid state
    auto& gridState = gridStateEntity_.get_mut<GridState>();
    gridState.liveCellCount++;
    
    return entity;
}

void GameOfLifeSimulation::destroyCell(int32_t x, int32_t y) {
    Position pos(x, y);
    auto it = spatialIndex_.find(pos);
    if (it != spatialIndex_.end()) {
        it->second.destruct();
        spatialIndex_.erase(it);
        
        // Update grid state
        auto& gridState = gridStateEntity_.get_mut<GridState>();
        if (gridState.liveCellCount > 0) {
            gridState.liveCellCount--;
        }
    }
}

bool GameOfLifeSimulation::isCellAlive(int32_t x, int32_t y) const {
    Position pos(x, y);
    auto it = spatialIndex_.find(pos);
    return it != spatialIndex_.end() && it->second.is_alive();
}

flecs::entity GameOfLifeSimulation::getCellAt(int32_t x, int32_t y) const {
    Position pos(x, y);
    auto it = spatialIndex_.find(pos);
    if (it != spatialIndex_.end()) {
        return it->second;
    }
    return flecs::entity(); // Return invalid entity
}

void GameOfLifeSimulation::step() {
    auto stepStart = std::chrono::high_resolution_clock::now();
    
    // Save current state before modifying
    std::set<Position> currentlyAlive;
    liveCellQuery_.each([&](flecs::entity, Position& pos, Cell&) {
        currentlyAlive.insert(pos);
    });
    
    // Count neighbors for all positions that could be affected
    std::unordered_map<Position, int> neighborCounts;
    for (const auto& cellPos : currentlyAlive) {
        auto neighbors = getNeighborPositions(cellPos.x, cellPos.y);
        for (const auto& neighborPos : neighbors) {
            neighborCounts[neighborPos]++;
        }
    }
    
    // Apply Conway's rules to determine next generation
    std::set<Position> nextGeneration;
    for (const auto& [pos, count] : neighborCounts) {
        bool isCurrentlyAlive = (currentlyAlive.find(pos) != currentlyAlive.end());
        
        if (isCurrentlyAlive) {
            // Live cell survives with 2 or 3 neighbors
            if (count == 2 || count == 3) {
                nextGeneration.insert(pos);
            }
            // Otherwise it dies (underpopulation or overpopulation)
        } else {
            // Dead cell becomes alive with exactly 3 neighbors
            if (count == 3 && isValidPosition(pos.x, pos.y)) {
                nextGeneration.insert(pos);
            }
        }
    }
    
    // Update the grid atomically
    clear();
    for (const auto& pos : nextGeneration) {
        createCell(pos.x, pos.y);
    }
    
    updatePerformanceMetrics();
    
    // Increment generation
    auto& gridState = gridStateEntity_.get_mut<GridState>();
    gridState.generation++;
    
    lastStepTime_ = stepStart;
}

void GameOfLifeSimulation::reset() {
    clear();
    auto& gridState = gridStateEntity_.get_mut<GridState>();
    gridState.generation = 0;
}

void GameOfLifeSimulation::clear() {
    // Destroy all live cells
    liveCellQuery_.each([&](flecs::entity entity, Position&, Cell&) {
        entity.destruct();
    });
    
    // Clear birth candidates
    birthCandidateQuery_.each([&](flecs::entity entity, Position&, BirthCandidate&) {
        entity.destruct();
    });
    
    spatialIndex_.clear();
    
    auto& gridState = gridStateEntity_.get_mut<GridState>();
    gridState.liveCellCount = 0;
}

uint32_t GameOfLifeSimulation::getCellCount() const {
    const auto& gridState = gridStateEntity_.get<GridState>();
    return gridState.liveCellCount;
}

uint32_t GameOfLifeSimulation::getGeneration() const {
    const auto& gridState = gridStateEntity_.get<GridState>();
    return gridState.generation;
}

size_t GameOfLifeSimulation::getMemoryUsage() const {
    const auto& metrics = performanceEntity_.get<PerformanceMetrics>();
    return metrics.memoryUsage;
}

uint8_t GameOfLifeSimulation::getNeighborCount(int32_t x, int32_t y) const {
    uint8_t count = 0;
    auto neighborPositions = getNeighborPositions(x, y);
    
    for (const auto& neighborPos : neighborPositions) {
        if (isCellAlive(neighborPos.x, neighborPos.y)) {
            count++;
        }
    }
    
    return count;
}

void GameOfLifeSimulation::updateNeighborCounts() {
    neighborCountSystem();
}

std::vector<flecs::entity> GameOfLifeSimulation::getAllCells() const {
    std::vector<flecs::entity> cells;
    liveCellQuery_.each([&](flecs::entity entity, Position&, Cell&) {
        cells.push_back(entity);
    });
    return cells;
}

std::vector<flecs::entity> GameOfLifeSimulation::getCellsInRegion(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) const {
    std::vector<flecs::entity> cells;
    liveCellQuery_.each([&](flecs::entity entity, Position& pos, Cell&) {
        if (pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY) {
            cells.push_back(entity);
        }
    });
    return cells;
}

std::vector<flecs::entity> GameOfLifeSimulation::getCellsWithNeighborCount(uint8_t count) const {
    std::vector<flecs::entity> cells;
    liveCellQuery_.each([&](flecs::entity entity, Position&, Cell& cell) {
        if (cell.neighborCount == count) {
            cells.push_back(entity);
        }
    });
    return cells;
}

void GameOfLifeSimulation::neighborCountSystem() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Reset neighbor counts for all live cells
    liveCellQuery_.each([](flecs::entity, Position&, Cell& cell) {
        cell.neighborCount = 0;
    });
    
    // Clear previous birth candidates
    birthCandidateQuery_.each([](flecs::entity entity, Position&, BirthCandidate&) {
        entity.destruct();
    });
    
    // Count neighbors for each live cell and track potential birth positions
    std::unordered_map<Position, uint8_t> potentialBirths;
    
    // For each live cell, count how many live neighbors it has
    liveCellQuery_.each([&](flecs::entity entity, Position& pos, Cell& cell) {
        auto neighborPositions = getNeighborPositions(pos.x, pos.y);
        
        for (const auto& neighborPos : neighborPositions) {
            if (isCellAlive(neighborPos.x, neighborPos.y)) {
                // This neighbor position has a live cell
                cell.neighborCount++;
            }
        }
    });
    
    // For each live cell, contribute to potential births at empty neighbor positions
    liveCellQuery_.each([&](flecs::entity, Position& pos, Cell&) {
        auto neighborPositions = getNeighborPositions(pos.x, pos.y);
        
        for (const auto& neighborPos : neighborPositions) {
            if (!isCellAlive(neighborPos.x, neighborPos.y) && isValidPosition(neighborPos.x, neighborPos.y)) {
                // This neighbor position is empty, increment its potential birth count
                potentialBirths[neighborPos]++;
            }
        }
    });
    
    // Create birth candidate entities for positions with exactly 3 neighbors
    for (const auto& [pos, count] : potentialBirths) {
        if (count == 3 && isValidPosition(pos.x, pos.y)) {
            world_.entity()
                .set<Position>(pos)
                .set<BirthCandidate>(BirthCandidate(count));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto& metrics = performanceEntity_.get_mut<PerformanceMetrics>();
    metrics.neighborCountTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void GameOfLifeSimulation::ruleEvaluationSystem() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Apply Conway's Game of Life rules to live cells
    liveCellQuery_.each([](flecs::entity, Position&, Cell& cell) {
        // Cell survives if it has 2 or 3 neighbors
        cell.willLive = (cell.neighborCount == 2 || cell.neighborCount == 3);
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto& metrics = performanceEntity_.get_mut<PerformanceMetrics>();
    metrics.ruleEvalTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void GameOfLifeSimulation::lifecycleSystem() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Collect entities to destroy and positions for new births
    std::vector<flecs::entity> entitiesToDestroy;
    std::vector<Position> positionsToCreate;
    
    // Check which live cells should die
    liveCellQuery_.each([&](flecs::entity entity, Position& pos, Cell& cell) {
        if (!cell.willLive) {
            entitiesToDestroy.push_back(entity);
        }
    });
    
    // Check birth candidates
    birthCandidateQuery_.each([&](flecs::entity entity, Position& pos, BirthCandidate&) {
        positionsToCreate.push_back(pos);
        entity.destruct(); // Remove birth candidate entity
    });
    
    // Destroy dying cells
    for (auto entity : entitiesToDestroy) {
        const auto& pos = entity.get<Position>();
        spatialIndex_.erase(pos);
        entity.destruct();
    }
    
    // Create new cells
    for (const auto& pos : positionsToCreate) {
        auto entity = world_.entity()
            .set<Position>(pos)
            .set<Cell>({});
        spatialIndex_[pos] = entity;
    }
    
    // Update grid state
    auto& gridState = gridStateEntity_.get_mut<GridState>();
    gridState.liveCellCount = static_cast<uint32_t>(spatialIndex_.size());
    
    auto end = std::chrono::high_resolution_clock::now();
    auto& metrics = performanceEntity_.get_mut<PerformanceMetrics>();
    metrics.lifecycleTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void GameOfLifeSimulation::updatePerformanceMetrics() {
    auto& metrics = performanceEntity_.get_mut<PerformanceMetrics>();
    metrics.entityCount = static_cast<uint32_t>(spatialIndex_.size());
    
    // Simple memory usage estimation
    metrics.memoryUsage = spatialIndex_.size() * (sizeof(Position) + sizeof(Cell) + sizeof(flecs::entity) + 64);
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    metrics.lastUpdateTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastStepTime_).count();
}

bool GameOfLifeSimulation::isValidPosition(int32_t x, int32_t y) const {
    return config_.isPointInBounds(x, y);
}

Position GameOfLifeSimulation::wrapPosition(int32_t x, int32_t y) const {
    if (!config_.getWrapEdges()) {
        return Position(x, y);
    }
    
    int32_t wrappedX = x;
    int32_t wrappedY = y;
    
    int32_t gridWidth = config_.getGridWidth();
    int32_t gridHeight = config_.getGridHeight();
    
    if (x < config_.getGridMinX()) {
        wrappedX = config_.getGridMaxX() - (config_.getGridMinX() - x - 1) % gridWidth;
    } else if (x > config_.getGridMaxX()) {
        wrappedX = config_.getGridMinX() + (x - config_.getGridMaxX() - 1) % gridWidth;
    }
    
    if (y < config_.getGridMinY()) {
        wrappedY = config_.getGridMaxY() - (config_.getGridMinY() - y - 1) % gridHeight;
    } else if (y > config_.getGridMaxY()) {
        wrappedY = config_.getGridMinY() + (y - config_.getGridMaxY() - 1) % gridHeight;
    }
    
    return Position(wrappedX, wrappedY);
}

std::vector<Position> GameOfLifeSimulation::getNeighborPositions(int32_t x, int32_t y) const {
    std::vector<Position> neighbors;
    neighbors.reserve(8);
    
    for (const auto& [dx, dy] : NEIGHBOR_OFFSETS) {
        int32_t neighborX = x + dx;
        int32_t neighborY = y + dy;
        
        if (config_.getWrapEdges()) {
            auto wrappedPos = wrapPosition(neighborX, neighborY);
            neighbors.push_back(wrappedPos);
        } else if (isValidPosition(neighborX, neighborY)) {
            neighbors.emplace_back(neighborX, neighborY);
        }
    }
    
    return neighbors;
}

void GameOfLifeSimulation::rebuildSpatialIndex() {
    spatialIndex_.clear();
    liveCellQuery_.each([&](flecs::entity entity, Position& pos, Cell&) {
        spatialIndex_[pos] = entity;
    });
}

} // namespace flecs_gol