#pragma once

#include "core/GameOfLifeSimulation.h"
#include "core/GameConfig.h"
#include <chrono>
#include <memory>
#include <vector>
#include <functional>

enum class SimulationState {
    Stopped,
    Running,
    Paused,
    Stepping
};

struct SimulationStats {
    std::uint64_t generation{0};
    std::size_t livingCells{0};
    double actualFps{0.0};
    std::chrono::milliseconds lastStepTime{0};
    bool isStable{false};
};

class SimulationController {
public:
    explicit SimulationController(const GameConfig& config = GameConfig{});
    
    // Core control interface
    void start();
    void pause();
    void stop();
    void step();
    void reset();
    
    // State queries
    SimulationState getState() const { return state_; }
    const SimulationStats& getStats() const { return stats_; }
    const GameConfig& getConfig() const { return simulation_->getConfig(); }
    
    // Configuration
    void setConfig(const GameConfig& config);
    void loadPattern(const std::string& patternFile);
    void setDefaultPattern(const std::string& patternFile);
    
    // Headless operation (for testing and Unity integration)
    void runHeadless(std::uint64_t maxGenerations = 1000);
    void setStepCallback(std::function<void(const SimulationStats&)> callback);
    
    // Cell queries (for view layer)
    bool isCellAlive(std::int32_t x, std::int32_t y) const;
    std::size_t getLivingCellCount() const;
    std::vector<std::pair<std::int32_t, std::int32_t>> getLivingCells() const;
    
    // Cell manipulation (for testing and initial setup)
    void setCellAlive(std::int32_t x, std::int32_t y);
    
    // Timing control
    void setTargetFps(std::int32_t fps);
    bool shouldUpdate() const;
    void updateTiming();

private:
    std::unique_ptr<GameOfLifeSimulation> simulation_;
    SimulationState state_{SimulationState::Stopped};
    SimulationStats stats_;
    
    // Timing management
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::milliseconds targetFrameTime_;
    std::chrono::steady_clock::time_point lastFpsCalculation_;
    std::uint64_t frameCount_{0};
    
    // Stability detection
    std::vector<std::size_t> populationHistory_;
    std::size_t stableDetectionWindow_{10};
    
    // Callbacks
    std::function<void(const SimulationStats&)> stepCallback_;
    
    // Pattern management
    std::vector<std::pair<std::int32_t, std::int32_t>> defaultPattern_;
    
    // Helper methods
    void updateStats();
    void checkStability();
    void calculateFps();
};