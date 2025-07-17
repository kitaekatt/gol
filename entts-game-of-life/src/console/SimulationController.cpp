#include "console/SimulationController.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <thread>

using json = nlohmann::json;

SimulationController::SimulationController(const GameConfig& config) 
    : simulation_(std::make_unique<GameOfLifeSimulation>(config)) {
    
    setTargetFps(config.getTargetFps());
    lastUpdate_ = std::chrono::steady_clock::now();
    lastFpsCalculation_ = lastUpdate_;
    
    populationHistory_.reserve(stableDetectionWindow_);
    updateStats();
}

void SimulationController::start() {
    if (state_ == SimulationState::Stopped || state_ == SimulationState::Paused) {
        state_ = SimulationState::Running;
        lastUpdate_ = std::chrono::steady_clock::now();
        lastFpsCalculation_ = lastUpdate_;
        frameCount_ = 0;
    }
}

void SimulationController::pause() {
    if (state_ == SimulationState::Running) {
        state_ = SimulationState::Paused;
    }
}

void SimulationController::stop() {
    state_ = SimulationState::Stopped;
    reset();
}

void SimulationController::step() {
    auto stepStart = std::chrono::steady_clock::now();
    
    bool hasChanges = simulation_->step();
    updateStats();
    checkStability();
    
    // Auto-pause if no changes occurred (simulation is static)
    if (!hasChanges && state_ == SimulationState::Running) {
        pause();
    }
    
    auto stepEnd = std::chrono::steady_clock::now();
    stats_.lastStepTime = std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart);
    
    if (stepCallback_) {
        stepCallback_(stats_);
    }
    
    // Update frame timing
    ++frameCount_;
    lastUpdate_ = stepEnd;
}

void SimulationController::reset() {
    simulation_->reset();
    stats_ = SimulationStats{};
    populationHistory_.clear();
    frameCount_ = 0;
    
    // Restore default pattern if one is set
    for (const auto& [x, y] : defaultPattern_) {
        simulation_->setCellAlive(x, y);
    }
    
    updateStats();
}

void SimulationController::setConfig(const GameConfig& config) {
    simulation_->setConfig(config);
    setTargetFps(config.getTargetFps());
    stableDetectionWindow_ = config.getStableDetectionCycles();
    reset();
}

void SimulationController::loadPattern(const std::string& patternFile) {
    std::ifstream file(patternFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open pattern file: " + patternFile);
    }
    
    json patternJson;
    file >> patternJson;
    file.close();
    
    // Reset simulation before loading pattern
    reset();
    
    // Load pattern data
    if (patternJson.contains("cells")) {
        for (const auto& cell : patternJson["cells"]) {
            if (cell.contains("x") && cell.contains("y")) {
                std::int32_t x = cell["x"];
                std::int32_t y = cell["y"];
                simulation_->setCellAlive(x, y);
            }
        }
    }
    
    updateStats();
}

void SimulationController::setDefaultPattern(const std::string& patternFile) {
    std::ifstream file(patternFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open default pattern file: " + patternFile);
    }
    
    json patternJson;
    file >> patternJson;
    file.close();
    
    // Clear existing default pattern
    defaultPattern_.clear();
    
    // Store pattern data
    if (patternJson.contains("cells")) {
        for (const auto& cell : patternJson["cells"]) {
            if (cell.contains("x") && cell.contains("y")) {
                std::int32_t x = cell["x"];
                std::int32_t y = cell["y"];
                defaultPattern_.emplace_back(x, y);
            }
        }
    }
}

void SimulationController::setCellAlive(std::int32_t x, std::int32_t y) {
    simulation_->setCellAlive(x, y);
    updateStats();
}

void SimulationController::runHeadless(std::uint64_t maxGenerations) {
    start();
    
    while (state_ == SimulationState::Running && stats_.generation < maxGenerations) {
        if (shouldUpdate()) {
            step();
            updateTiming();
            
            // Check for early termination conditions
            if (stats_.isStable && simulation_->getConfig().getAutoPauseOnStable()) {
                pause();
                break;
            }
            
            if (stats_.livingCells == 0) {
                pause();
                break;
            }
        }
        
        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    stop();
}

void SimulationController::setStepCallback(std::function<void(const SimulationStats&)> callback) {
    stepCallback_ = std::move(callback);
}

bool SimulationController::isCellAlive(std::int32_t x, std::int32_t y) const {
    return simulation_->isCellAlive(x, y);
}

std::size_t SimulationController::getLivingCellCount() const {
    return simulation_->getLivingCellCount();
}

std::vector<std::pair<std::int32_t, std::int32_t>> SimulationController::getLivingCells() const {
    std::vector<std::pair<std::int32_t, std::int32_t>> cells;
    cells.reserve(simulation_->getLivingCellCount());
    
    const auto& registry = simulation_->getRegistry();
    auto view = registry.view<Position, Cell>();
    
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        const auto& cell = view.get<Cell>(entity);
        
        if (cell.alive) {
            cells.emplace_back(pos.x, pos.y);
        }
    }
    
    return cells;
}

void SimulationController::setTargetFps(std::int32_t fps) {
    if (fps > 0) {
        targetFrameTime_ = std::chrono::milliseconds(1000 / fps);
    } else {
        targetFrameTime_ = std::chrono::milliseconds(0);
    }
}

bool SimulationController::shouldUpdate() const {
    if (state_ != SimulationState::Running) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate_);
    
    return elapsed >= targetFrameTime_;
}

void SimulationController::updateTiming() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFpsCalculation_);
    
    // Calculate FPS every second
    if (elapsed >= std::chrono::milliseconds(1000)) {
        calculateFps();
        lastFpsCalculation_ = now;
        frameCount_ = 0;
    }
}

void SimulationController::updateStats() {
    stats_.generation = simulation_->getGenerationCount();
    stats_.livingCells = simulation_->getLivingCellCount();
}

void SimulationController::checkStability() {
    populationHistory_.push_back(stats_.livingCells);
    
    // Keep only the last N entries
    if (populationHistory_.size() > stableDetectionWindow_) {
        populationHistory_.erase(populationHistory_.begin());
    }
    
    // Check for stability (population hasn't changed)
    if (populationHistory_.size() >= stableDetectionWindow_) {
        bool stable = std::all_of(populationHistory_.begin(), populationHistory_.end(),
            [this](std::size_t pop) { return pop == populationHistory_[0]; });
        
        stats_.isStable = stable;
    } else {
        stats_.isStable = false;
    }
}

void SimulationController::calculateFps() {
    if (frameCount_ > 0) {
        stats_.actualFps = static_cast<double>(frameCount_);
    } else {
        stats_.actualFps = 0.0;
    }
}