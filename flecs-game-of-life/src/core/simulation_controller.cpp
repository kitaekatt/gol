#include <flecs_gol/simulation_controller.h>
#include <fstream>
#include <thread>
#include <algorithm>
#include <iostream>

namespace flecs_gol {

SimulationController::SimulationController(const GameConfig& config)
    : config_(config)
    , targetFrameTime_(1000 / config.getTargetFPS())
    , lastFrameTime_(std::chrono::high_resolution_clock::now())
    , startTime_(lastFrameTime_) {
    
    simulation_ = std::make_unique<GameOfLifeSimulation>(config_);
    
    // Initialize performance tracking
    stepTimes_.fill(0);
    
    updateState();
}

SimulationController::~SimulationController() {
    stop();
}

void SimulationController::start() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    
    if (!currentState_.isRunning) {
        currentState_.isRunning = true;
        currentState_.isPaused = false;
        shouldStop_ = false;
        threadRunning_ = true;
        
        simulationThread_ = std::thread(&SimulationController::simulationLoop, this);
        notifyStateChange();
    }
}

void SimulationController::pause() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    
    if (currentState_.isRunning && !currentState_.isPaused) {
        currentState_.isPaused = true;
        notifyStateChange();
    }
}

void SimulationController::resume() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    
    if (currentState_.isRunning && currentState_.isPaused) {
        currentState_.isPaused = false;
        notifyStateChange();
    }
}

void SimulationController::stop() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        shouldStop_ = true;
        currentState_.isRunning = false;
        currentState_.isPaused = false;
    }
    
    if (simulationThread_.joinable()) {
        simulationThread_.join();
    }
    
    threadRunning_ = false;
    notifyStateChange();
}

void SimulationController::step() {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    
    auto stepStart = std::chrono::high_resolution_clock::now();
    
    simulation_->step();
    
    auto stepEnd = std::chrono::high_resolution_clock::now();
    auto stepTime = std::chrono::duration_cast<std::chrono::microseconds>(stepEnd - stepStart).count();
    
    // Update performance tracking
    stepTimes_[stepTimeIndex_] = stepTime;
    stepTimeIndex_ = (stepTimeIndex_ + 1) % PERFORMANCE_HISTORY_SIZE;
    
    updateState();
    
    if (generationCallback_) {
        generationCallback_(currentState_.generation);
    }
    
    if (patternDetectionEnabled_) {
        detectPatterns();
    }
}

void SimulationController::reset() {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    
    simulation_->reset();
    
    // Restore initial pattern if we have one
    if (hasInitialPattern_) {
        // Load cells from initial pattern
        if (initialPattern_.contains("cells") && initialPattern_["cells"].is_array()) {
            for (const auto& cell : initialPattern_["cells"]) {
                if (cell.contains("x") && cell.contains("y")) {
                    int32_t x = cell["x"];
                    int32_t y = cell["y"];
                    simulation_->createCell(x, y);
                }
            }
        }
    }
    
    updateState();
    
    // Reset performance tracking
    stepTimes_.fill(0);
    stepTimeIndex_ = 0;
    generationHistory_.clear();
    detectedPatterns_.clear();
    
    notifyStateChange();
}

void SimulationController::loadPattern(const std::string& patternFile) {
    try {
        std::ifstream file(patternFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open pattern file: " + patternFile);
        }
        
        nlohmann::json patternJson;
        file >> patternJson;
        
        loadPatternFromJson(patternJson);
    } catch (const std::exception& e) {
        std::cerr << "Error loading pattern: " << e.what() << std::endl;
        throw;
    }
}

void SimulationController::loadPatternFromJson(const nlohmann::json& patternJson) {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    
    // Store the initial pattern for reset functionality
    initialPattern_ = patternJson;
    hasInitialPattern_ = true;
    
    // Clear existing cells
    simulation_->clear();
    
    // Load cells from pattern
    if (patternJson.contains("cells") && patternJson["cells"].is_array()) {
        for (const auto& cell : patternJson["cells"]) {
            if (cell.contains("x") && cell.contains("y")) {
                int32_t x = cell["x"];
                int32_t y = cell["y"];
                simulation_->createCell(x, y);
            }
        }
    }
    
    updateState();
    notifyStateChange();
}

void SimulationController::setTargetFPS(uint32_t fps) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    
    config_.setTargetFPS(fps);
    targetFrameTime_ = std::chrono::milliseconds(1000 / fps);
}

void SimulationController::setAutoStep(bool enabled) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    autoStep_ = enabled;
}

SimulationState SimulationController::getState() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentState_;
}

std::vector<CellData> SimulationController::getCellsInRegion(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) const {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    
    std::vector<CellData> cells;
    auto entities = simulation_->getCellsInRegion(minX, maxX, minY, maxY);
    
    for (const auto& entity : entities) {
        if (true) { const auto& pos = entity.get<Position>();
            cells.emplace_back(pos.x, pos.y);
        }
    }
    
    return cells;
}

std::vector<CellData> SimulationController::getAllCells() const {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    
    std::vector<CellData> cells;
    auto entities = simulation_->getAllCells();
    
    for (const auto& entity : entities) {
        if (true) { const auto& pos = entity.get<Position>();
            cells.emplace_back(pos.x, pos.y);
        }
    }
    
    return cells;
}

const GameConfig& SimulationController::getConfig() const {
    return config_;
}

void SimulationController::setGenerationCallback(GenerationCallback callback) {
    generationCallback_ = std::move(callback);
}

void SimulationController::setStateChangeCallback(StateChangeCallback callback) {
    stateChangeCallback_ = std::move(callback);
}

void SimulationController::setPatternDetectedCallback(PatternDetectedCallback callback) {
    patternDetectedCallback_ = std::move(callback);
}

bool SimulationController::isValidPosition(int32_t x, int32_t y) const {
    return config_.isPointInBounds(x, y);
}

void SimulationController::addCell(int32_t x, int32_t y) {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    simulation_->createCell(x, y);
    updateState();
}

void SimulationController::removeCell(int32_t x, int32_t y) {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    simulation_->destroyCell(x, y);
    updateState();
}

void SimulationController::clearGrid() {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    simulation_->clear();
    updateState();
    notifyStateChange();
}

void SimulationController::enablePatternDetection(bool enabled) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    patternDetectionEnabled_ = enabled;
    
    if (!enabled) {
        generationHistory_.clear();
        detectedPatterns_.clear();
    }
}

bool SimulationController::isPatternDetectionEnabled() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return patternDetectionEnabled_;
}

void SimulationController::simulationLoop() {
    while (!shouldStop_) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            if (currentState_.isPaused || !autoStep_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        }
        
        // Perform simulation step
        step();
        
        // Frame rate limiting
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        
        if (frameDuration < targetFrameTime_) {
            std::this_thread::sleep_for(targetFrameTime_ - frameDuration);
        }
        
        // Update FPS calculation
        auto actualFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - frameStart);
        
        if (actualFrameDuration.count() > 0) {
            std::lock_guard<std::mutex> lock(stateMutex_);
            currentState_.actualFPS = 1000.0 / actualFrameDuration.count();
        }
    }
}

void SimulationController::updateState() {
    currentState_.generation = simulation_->getGeneration();
    currentState_.liveCellCount = simulation_->getCellCount();
    currentState_.memoryUsage = simulation_->getMemoryUsage();
    
    // Calculate average step time
    uint64_t totalTime = 0;
    size_t validSamples = 0;
    
    for (uint64_t time : stepTimes_) {
        if (time > 0) {
            totalTime += time;
            validSamples++;
        }
    }
    
    if (validSamples > 0) {
        currentState_.averageStepTimeMicros = totalTime / validSamples;
    }
    
    if (!stepTimes_.empty() && stepTimes_[(stepTimeIndex_ + PERFORMANCE_HISTORY_SIZE - 1) % PERFORMANCE_HISTORY_SIZE] > 0) {
        currentState_.lastStepTimeMicros = stepTimes_[(stepTimeIndex_ + PERFORMANCE_HISTORY_SIZE - 1) % PERFORMANCE_HISTORY_SIZE];
    }
}

void SimulationController::notifyStateChange() {
    if (stateChangeCallback_) {
        stateChangeCallback_(currentState_);
    }
}

void SimulationController::detectPatterns() {
    // Simple pattern detection based on cell count periodicity
    generationHistory_.push_back(currentState_.liveCellCount);
    
    // Keep only recent history
    constexpr size_t MAX_HISTORY = 200;
    if (generationHistory_.size() > MAX_HISTORY) {
        generationHistory_.erase(generationHistory_.begin());
    }
    
    // Look for periodic patterns
    if (generationHistory_.size() > 20) {
        for (uint32_t period = 1; period <= 50 && period < generationHistory_.size() / 2; ++period) {
            bool isPeriodic = true;
            size_t checkLength = std::min(static_cast<size_t>(period * 3), generationHistory_.size() - period);
            
            for (size_t i = 0; i < checkLength; ++i) {
                if (generationHistory_[generationHistory_.size() - 1 - i] != 
                    generationHistory_[generationHistory_.size() - 1 - i - period]) {
                    isPeriodic = false;
                    break;
                }
            }
            
            if (isPeriodic) {
                std::string patternKey = "period_" + std::to_string(period);
                if (detectedPatterns_.find(patternKey) == detectedPatterns_.end()) {
                    detectedPatterns_[patternKey] = currentState_.generation;
                    
                    if (patternDetectedCallback_) {
                        std::string patternName;
                        switch (period) {
                            case 1: patternName = "Still Life"; break;
                            case 2: patternName = "Blinker"; break;
                            case 3: patternName = "Period-3 Oscillator"; break;
                            default: patternName = "Period-" + std::to_string(period) + " Oscillator"; break;
                        }
                        patternDetectedCallback_(patternName, period);
                    }
                }
                break;
            }
        }
    }
}

} // namespace flecs_gol