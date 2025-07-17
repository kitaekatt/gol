#include "core/GameOfLifeSimulation.h"
#include <algorithm>
#include <vector>
#include <unordered_set>

GameOfLifeSimulation::GameOfLifeSimulation(const GameConfig& config) 
    : config_(config) {
}

void GameOfLifeSimulation::setCellAlive(std::int32_t x, std::int32_t y) {
    if (!isValidPosition(x, y)) {
        return;
    }
    
    Position pos = normalizePosition(x, y);
    
    // Check if entity already exists at this position
    auto it = spatialIndex_.find(pos);
    if (it != spatialIndex_.end()) {
        // Entity already exists, just ensure it's alive
        auto& cell = registry_.get<Cell>(it->second);
        cell.alive = true;
        return;
    }
    
    // Create new entity
    auto entity = registry_.create();
    registry_.emplace<Position>(entity, pos);
    registry_.emplace<Cell>(entity, true);
    
    // Add to spatial index
    spatialIndex_[pos] = entity;
}

void GameOfLifeSimulation::setCellDead(std::int32_t x, std::int32_t y) {
    Position pos = normalizePosition(x, y);
    
    auto it = spatialIndex_.find(pos);
    if (it != spatialIndex_.end()) {
        // Remove from registry and spatial index
        registry_.destroy(it->second);
        spatialIndex_.erase(it);
    }
}

bool GameOfLifeSimulation::isCellAlive(std::int32_t x, std::int32_t y) const {
    Position pos = normalizePosition(x, y);
    
    auto it = spatialIndex_.find(pos);
    if (it == spatialIndex_.end()) {
        return false;
    }
    
    const auto& cell = registry_.get<Cell>(it->second);
    return cell.alive;
}

bool GameOfLifeSimulation::step() {
    // Store state before changes
    auto previousCellCount = spatialIndex_.size();
    auto previousCells = spatialIndex_; // Copy for comparison
    
    updateNeighborCounts();
    applyConwayRules();
    cleanupDeadCells();
    ++generationCount_;
    
    // Check if any changes occurred
    if (spatialIndex_.size() != previousCellCount) {
        return true; // Cell count changed
    }
    
    // Check if cell positions changed
    for (const auto& [pos, entity] : spatialIndex_) {
        if (previousCells.find(pos) == previousCells.end()) {
            return true; // New cell position
        }
    }
    
    return false; // No changes
}

void GameOfLifeSimulation::reset() {
    registry_.clear();
    spatialIndex_.clear();
    generationCount_ = 0;
}

std::size_t GameOfLifeSimulation::getLivingCellCount() const {
    return spatialIndex_.size();
}

std::uint8_t GameOfLifeSimulation::getNeighborCount(std::int32_t x, std::int32_t y) const {
    return calculateNeighborCount(x, y);
}

entt::entity GameOfLifeSimulation::getEntityAt(std::int32_t x, std::int32_t y) const {
    Position pos = normalizePosition(x, y);
    
    auto it = spatialIndex_.find(pos);
    return it != spatialIndex_.end() ? it->second : entt::null;
}

void GameOfLifeSimulation::setConfig(const GameConfig& config) {
    config_ = config;
    // Reset simulation when config changes
    reset();
}

bool GameOfLifeSimulation::isValidPosition(std::int32_t x, std::int32_t y) const {
    if (config_.getWrapEdges()) {
        return true; // All positions are valid with wrapping
    }
    
    return x >= 0 && x < config_.getGridWidth() && 
           y >= 0 && y < config_.getGridHeight();
}

Position GameOfLifeSimulation::normalizePosition(std::int32_t x, std::int32_t y) const {
    if (!config_.getWrapEdges()) {
        return Position(x, y);
    }
    
    // Wrap coordinates to grid boundaries
    std::int32_t wrappedX = ((x % config_.getGridWidth()) + config_.getGridWidth()) % config_.getGridWidth();
    std::int32_t wrappedY = ((y % config_.getGridHeight()) + config_.getGridHeight()) % config_.getGridHeight();
    
    return Position(wrappedX, wrappedY);
}

std::uint8_t GameOfLifeSimulation::calculateNeighborCount(std::int32_t x, std::int32_t y) const {
    std::uint8_t count = 0;
    
    for (const auto& [dx, dy] : neighborOffsets) {
        std::int32_t neighborX = x + dx;
        std::int32_t neighborY = y + dy;
        
        if (config_.getWrapEdges()) {
            // Normalize neighbor position with wrapping
            Position neighborPos = normalizePosition(neighborX, neighborY);
            if (spatialIndex_.find(neighborPos) != spatialIndex_.end()) {
                ++count;
            }
        } else {
            // Check bounds for non-wrapping grid
            if (neighborX >= 0 && neighborX < config_.getGridWidth() &&
                neighborY >= 0 && neighborY < config_.getGridHeight()) {
                Position neighborPos(neighborX, neighborY);
                if (spatialIndex_.find(neighborPos) != spatialIndex_.end()) {
                    ++count;
                }
            }
        }
    }
    
    return count;
}

void GameOfLifeSimulation::updateNeighborCounts() {
    // Update neighbor counts for all living cells
    auto view = registry_.view<Position, Cell>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        auto& cell = view.get<Cell>(entity);
        cell.neighborCount = calculateNeighborCount(pos.x, pos.y);
    }
}

void GameOfLifeSimulation::applyConwayRules() {
    std::vector<Position> cellsToCreate;
    std::vector<entt::entity> cellsToDestroy;
    
    // Check all cells that might be affected
    std::unordered_set<Position> cellsToCheck;
    
    // Add all living cells and their neighbors to check list
    auto view = registry_.view<Position, Cell>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        cellsToCheck.insert(pos);
        
        // Add all neighbor positions
        for (const auto& [dx, dy] : neighborOffsets) {
            std::int32_t neighborX = pos.x + dx;
            std::int32_t neighborY = pos.y + dy;
            if (isValidPosition(neighborX, neighborY)) {
                Position neighborPos = normalizePosition(neighborX, neighborY);
                cellsToCheck.insert(neighborPos);
            }
        }
    }
    
    // Apply Conway's rules to each cell
    for (const auto& pos : cellsToCheck) {
        bool isCurrentlyAlive = (spatialIndex_.find(pos) != spatialIndex_.end());
        std::uint8_t neighbors = calculateNeighborCount(pos.x, pos.y);
        
        if (isCurrentlyAlive) {
            // Rules for living cells
            if (neighbors < 2 || neighbors > 3) {
                // Cell dies (underpopulation or overpopulation)
                auto it = spatialIndex_.find(pos);
                if (it != spatialIndex_.end()) {
                    cellsToDestroy.push_back(it->second);
                }
            }
            // Cells with 2 or 3 neighbors survive (no action needed)
        } else {
            // Rules for dead cells
            if (neighbors == 3) {
                // Cell is born (reproduction)
                cellsToCreate.push_back(pos);
            }
        }
    }
    
    // Apply changes
    for (auto entity : cellsToDestroy) {
        const auto& pos = registry_.get<Position>(entity);
        spatialIndex_.erase(pos);
        registry_.destroy(entity);
    }
    
    for (const auto& pos : cellsToCreate) {
        auto entity = registry_.create();
        registry_.emplace<Position>(entity, pos);
        registry_.emplace<Cell>(entity, true);
        spatialIndex_[pos] = entity;
    }
}

void GameOfLifeSimulation::cleanupDeadCells() {
    // In this implementation, dead cells are immediately removed,
    // so no cleanup is needed. This method is here for future optimization
    // where we might keep dead cells for a while to avoid recreation costs.
}