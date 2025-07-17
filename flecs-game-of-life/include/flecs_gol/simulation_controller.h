#pragma once

#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/game_config.h>
#include <memory>
#include <chrono>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>
#include <unordered_map>

namespace flecs_gol {

// State information exposed by controller
struct SimulationState {
    bool isRunning = false;
    bool isPaused = false;
    uint32_t generation = 0;
    uint32_t liveCellCount = 0;
    double actualFPS = 0.0;
    size_t memoryUsage = 0;
    
    // Performance metrics
    uint64_t lastStepTimeMicros = 0;
    uint64_t averageStepTimeMicros = 0;
};

// Cell data for view consumption
struct CellData {
    int32_t x, y;
    bool isAlive = true;
    bool isNewBorn = false;  // For rendering effects
    bool isDying = false;    // For rendering effects
    
    CellData(int32_t x, int32_t y) : x(x), y(y) {}
};

// Callback types for event notifications
using GenerationCallback = std::function<void(uint32_t generation)>;
using StateChangeCallback = std::function<void(const SimulationState& state)>;
using PatternDetectedCallback = std::function<void(const std::string& patternName, uint32_t period)>;

class SimulationController {
public:
    explicit SimulationController(const GameConfig& config);
    ~SimulationController();
    
    // Simulation control - thread-safe
    void start();
    void pause();
    void resume();
    void stop();
    void step();  // Single step when paused
    void reset();
    
    // Configuration
    void loadPattern(const std::string& patternFile);
    void loadPatternFromJson(const nlohmann::json& patternJson);
    void setTargetFPS(uint32_t fps);
    void setAutoStep(bool enabled);
    
    // State queries - thread-safe, const access
    SimulationState getState() const;
    std::vector<CellData> getCellsInRegion(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) const;
    std::vector<CellData> getAllCells() const;
    const GameConfig& getConfig() const;
    
    // Callbacks for events
    void setGenerationCallback(GenerationCallback callback);
    void setStateChangeCallback(StateChangeCallback callback);
    void setPatternDetectedCallback(PatternDetectedCallback callback);
    
    // Utility methods
    bool isValidPosition(int32_t x, int32_t y) const;
    void addCell(int32_t x, int32_t y);
    void removeCell(int32_t x, int32_t y);
    void clearGrid();
    
    // Pattern detection
    void enablePatternDetection(bool enabled);
    bool isPatternDetectionEnabled() const;

private:
    // Internal simulation thread management
    void simulationLoop();
    void updateState();
    void notifyStateChange();
    void detectPatterns();
    
    // Thread-safe data access
    mutable std::mutex stateMutex_;
    mutable std::mutex simulationMutex_;
    
    // Core simulation
    std::unique_ptr<GameOfLifeSimulation> simulation_;
    GameConfig config_;
    
    // Controller state
    SimulationState currentState_;
    bool shouldStop_ = false;
    bool autoStep_ = true;
    
    // Timing control
    std::chrono::milliseconds targetFrameTime_;
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    std::chrono::high_resolution_clock::time_point startTime_;
    
    // Performance tracking
    static constexpr size_t PERFORMANCE_HISTORY_SIZE = 60;
    std::array<uint64_t, PERFORMANCE_HISTORY_SIZE> stepTimes_;
    size_t stepTimeIndex_ = 0;
    
    // Callbacks
    GenerationCallback generationCallback_;
    StateChangeCallback stateChangeCallback_;
    PatternDetectedCallback patternDetectedCallback_;
    
    // Pattern management
    nlohmann::json initialPattern_;
    bool hasInitialPattern_ = false;
    
    // Pattern detection
    bool patternDetectionEnabled_ = false;
    std::vector<uint32_t> generationHistory_;
    std::unordered_map<std::string, uint32_t> detectedPatterns_;
    
    // Thread management
    std::thread simulationThread_;
    std::atomic<bool> threadRunning_{false};
};

} // namespace flecs_gol