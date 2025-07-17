#include <flecs_gol/flecs_gol.h>
#include <flecs_gol/simulation_controller.h>
#include <flecs_gol/console_renderer.h>
#include <flecs_gol/console_input.h>
#include <iostream>
#include <chrono>
#include <csignal>
#include <atomic>
#include <thread>
#include <string>
#include <cstring>
#include <memory>

using namespace flecs_gol;

// Global state for signal handling
std::atomic<bool> g_shouldExit{false};

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_shouldExit = true;
    }
}

class ConsoleApplication {
public:
    ConsoleApplication() 
        : config_()
        , controller_(std::make_unique<SimulationController>(config_))
        , renderer_()
        , input_() {
        
        setupCallbacks();
        setupSignalHandlers();
    }
    
    ~ConsoleApplication() {
        cleanup();
    }
    
    bool initialize(int argc, char* argv[]) {
        // Parse command line arguments
        std::string configFile = "config/default.json";
        std::string patternFile;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--config" && i + 1 < argc) {
                configFile = argv[++i];
            } else if (arg == "--pattern" && i + 1 < argc) {
                patternFile = argv[++i];
            } else if (arg == "--help" || arg == "-h") {
                showUsage(argv[0]);
                return false;
            } else if (arg == "--headless") {
                headlessMode_ = true;
            } else if (arg == "--fps" && i + 1 < argc) {
                targetFPS_ = std::stoi(argv[++i]);
            }
        }
        
        // Load configuration
        try {
            auto loadedConfig = GameConfig::loadFromFile(configFile);
            if (loadedConfig.has_value()) {
                config_ = loadedConfig.value();
                std::cout << "Loaded configuration from: " << configFile << std::endl;
            } else {
                std::cout << "Using default configuration (could not load: " << configFile << ")" << std::endl;
            }
            
            if (targetFPS_ > 0) {
                config_.setTargetFPS(targetFPS_);
            }
            
            // Re-create controller with new config
            controller_ = std::make_unique<SimulationController>(config_);
            setupCallbacks();
            
        } catch (const std::exception& e) {
            std::cerr << "Error loading configuration: " << e.what() << std::endl;
            return false;
        }
        
        // Load initial pattern if specified
        if (!patternFile.empty()) {
            try {
                controller_->loadPattern(patternFile);
                std::cout << "Loaded pattern from: " << patternFile << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not load pattern file: " << e.what() << std::endl;
            }
        }
        
        // Initialize input if not in headless mode
        if (!headlessMode_) {
            input_.start();
        }
        
        return true;
    }
    
    void run() {
        if (headlessMode_) {
            runHeadless();
        } else {
            runInteractive();
        }
    }

private:
    void setupCallbacks() {
        // Set up controller callbacks
        controller_->setGenerationCallback([this](uint32_t generation) {
            if (generation % 100 == 0) {
                auto state = controller_->getState();
                std::cout << "Generation " << generation 
                         << " - Cells: " << state.liveCellCount 
                         << " - FPS: " << state.actualFPS << std::endl;
            }
        });
        
        // Set up input callbacks
        input_.setInputCallback([this](InputEvent event) {
            handleInputEvent(event);
        });
        
        input_.setPositionCallback([this](int32_t deltaX, int32_t deltaY) {
            renderer_.moveViewport(deltaX, deltaY);
        });
    }
    
    void setupSignalHandlers() {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
    }
    
    void runInteractive() {
        std::cout << "=== Conway's Game of Life ===" << std::endl;
        std::cout << "Press H for help, Q to quit" << std::endl;
        std::cout << "Simulation paused. Press SPACE to start." << std::endl;
        
        controller_->start();
        controller_->pause(); // Start paused
        
        auto lastRenderTime = std::chrono::high_resolution_clock::now();
        const auto renderInterval = std::chrono::milliseconds(50); // 20 FPS max for rendering
        
        while (!g_shouldExit && !shouldExit_) {
            auto now = std::chrono::high_resolution_clock::now();
            
            // Render at controlled rate
            if (now - lastRenderTime >= renderInterval) {
                renderer_.render(*controller_);
                lastRenderTime = now;
            }
            
            // Small sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        controller_->stop();
        std::cout << "\nSimulation stopped." << std::endl;
    }
    
    void runHeadless() {
        std::cout << "Running in headless mode..." << std::endl;
        
        controller_->start();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        const auto maxRunTime = std::chrono::seconds(30); // Run for 30 seconds max
        
        while (!g_shouldExit && !shouldExit_) {
            auto now = std::chrono::high_resolution_clock::now();
            
            if (now - startTime >= maxRunTime) {
                std::cout << "Maximum run time reached." << std::endl;
                break;
            }
            
            // Print status every 5 seconds
            static auto lastStatusTime = startTime;
            if (now - lastStatusTime >= std::chrono::seconds(5)) {
                auto state = controller_->getState();
                std::cout << "Status - Gen: " << state.generation 
                         << ", Cells: " << state.liveCellCount
                         << ", FPS: " << state.actualFPS 
                         << ", Memory: " << (state.memoryUsage / 1024) << "KB" << std::endl;
                lastStatusTime = now;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        controller_->stop();
        
        // Print final statistics
        auto state = controller_->getState();
        std::cout << "\nFinal Statistics:" << std::endl;
        std::cout << "  Generations: " << state.generation << std::endl;
        std::cout << "  Final cell count: " << state.liveCellCount << std::endl;
        std::cout << "  Average FPS: " << state.actualFPS << std::endl;
        std::cout << "  Memory usage: " << (state.memoryUsage / 1024) << "KB" << std::endl;
    }
    
    void handleInputEvent(InputEvent event) {
        switch (event) {
            case InputEvent::QUIT:
                shouldExit_ = true;
                break;
                
            case InputEvent::PAUSE_RESUME:
                {
                    auto state = controller_->getState();
                    if (state.isPaused) {
                        controller_->resume();
                    } else {
                        controller_->pause();
                    }
                }
                break;
                
            case InputEvent::STEP:
                controller_->step();
                break;
                
            case InputEvent::RESET:
                controller_->reset();
                break;
                
            case InputEvent::LOAD_GLIDER:
                loadPattern("examples/patterns/glider.json");
                break;
                
            case InputEvent::LOAD_BLINKER:
                loadPattern("examples/patterns/blinker.json");
                break;
                
            case InputEvent::LOAD_GOSPER_GUN:
                loadPattern("config/patterns/gosper_gun.json");
                break;
                
            case InputEvent::CLEAR_GRID:
                controller_->clearGrid();
                break;
                
            case InputEvent::ZOOM_IN:
                renderer_.zoomIn();
                break;
                
            case InputEvent::ZOOM_OUT:
                renderer_.zoomOut();
                break;
                
            case InputEvent::CENTER_VIEW:
                renderer_.resetViewport();
                break;
                
            case InputEvent::TOGGLE_AUTO_CENTER:
                {
                    auto config = renderer_.getViewportConfig();
                    renderer_.setAutoCenter(!config.autoCenter);
                }
                break;
                
            case InputEvent::TOGGLE_UI:
                {
                    auto config = renderer_.getViewportConfig();
                    renderer_.setShowUI(!config.showUI);
                }
                break;
                
            case InputEvent::TOGGLE_BORDER:
                {
                    auto config = renderer_.getViewportConfig();
                    renderer_.setShowBorder(!config.showBorder);
                }
                break;
                
            case InputEvent::TOGGLE_COLORS:
                {
                    auto config = renderer_.getViewportConfig();
                    renderer_.setColors(!config.useColors);
                }
                break;
                
            case InputEvent::HELP:
                showHelp();
                break;
                
            case InputEvent::SPEED_UP:
                {
                    auto currentFPS = controller_->getConfig().getTargetFPS();
                    controller_->setTargetFPS(std::min(currentFPS + 5, 120u));
                }
                break;
                
            case InputEvent::SLOW_DOWN:
                {
                    auto currentFPS = controller_->getConfig().getTargetFPS();
                    controller_->setTargetFPS(std::max(currentFPS - 5, 1u));
                }
                break;
                
            default:
                break;
        }
    }
    
    void loadPattern(const std::string& filename) {
        try {
            controller_->loadPattern(filename);
        } catch (const std::exception& e) {
            std::cerr << "Error loading pattern: " << e.what() << std::endl;
        }
    }
    
    void showHelp() {
        renderer_.clearScreen();
        std::cout << ConsoleInput::getHelpText();
        std::cout << "\nPress any key to continue...";
        std::cout.flush();
        
        // Wait for key press
        while (!input_.hasInput()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        input_.getNextInput(); // Consume the key press
        
        renderer_.clearScreen();
    }
    
    void showUsage(const char* programName) {
        std::cout << "Usage: " << programName << " [options]\n"
                  << "\nOptions:\n"
                  << "  --config FILE    Load configuration from FILE\n"
                  << "  --pattern FILE   Load initial pattern from FILE\n"
                  << "  --headless       Run without interactive display\n"
                  << "  --fps FPS        Set target simulation FPS\n"
                  << "  --help, -h       Show this help message\n"
                  << "\nExamples:\n"
                  << "  " << programName << " --pattern examples/patterns/glider.json\n"
                  << "  " << programName << " --headless --fps 60\n"
                  << "  " << programName << " --config config/performance_test.json\n"
                  << std::endl;
    }
    
    void cleanup() {
        input_.stop();
        controller_->stop();
        
        // Restore terminal state
        renderer_.clearScreen();
        std::cout << "\033[?25h"; // Show cursor
        std::cout.flush();
    }
    
    GameConfig config_;
    std::unique_ptr<SimulationController> controller_;
    ConsoleRenderer renderer_;
    ConsoleInput input_;
    
    bool shouldExit_ = false;
    bool headlessMode_ = false;
    uint32_t targetFPS_ = 0;
};

int main(int argc, char* argv[]) {
    try {
        ConsoleApplication app;
        
        if (!app.initialize(argc, argv)) {
            return 0; // Help was shown or initialization failed gracefully
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}