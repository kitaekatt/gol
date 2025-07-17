#include "console/SimulationController.h"
#include "console/ConsoleRenderer.h"
#include "console/ConsoleInput.h"
#include "core/GameConfig.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

class ConsoleApplication {
public:
    ConsoleApplication() 
        : controller_(loadDefaultConfig())
        , renderer_(createRenderConfig())
        , input_() {
        
        setupInputHandling();
        setupViewport();
    }
    
    void run() {
        std::cout << "Game of Life Console Application\n";
        std::cout << "Loading default pattern...\n";
        
        try {
            controller_.setDefaultPattern("config/glider.json");
            controller_.loadPattern("config/glider.json");
        } catch (const std::exception& e) {
            std::cout << "Could not load default pattern: " << e.what() << "\n";
            std::cout << "Starting with empty grid.\n";
        }
        
        std::cout << "Press any key to begin...\n";
        input_.getChar();
        
        mainLoop();
        cleanup();
    }

private:
    SimulationController controller_;
    ConsoleRenderer renderer_;
    ConsoleInput input_;
    bool running_{true};
    bool needsRender_{true}; // Force initial render
    
    void mainLoop() {
        while (running_ && input_.getState().running) {
            // Handle input
            input_.pollInput();
            
            // Update simulation if running
            if (controller_.shouldUpdate()) {
                controller_.step();
                controller_.updateTiming();
                needsRender_ = true; // Mark for render after simulation step
            }
            
            // Render frame only when needed
            if (needsRender_) {
                renderer_.render(controller_);
                needsRender_ = false;
            }
            
            // Small sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS max
        }
    }
    
    void setupInputHandling() {
        input_.setInputHandler([this](InputEvent event, const InputState& state) {
            handleInput(event, state);
        });
    }
    
    void setupViewport() {
        auto [termWidth, termHeight] = renderer_.getTerminalSize();
        
        // Reserve space for stats and controls
        std::int32_t gridHeight = termHeight - 6;
        renderer_.setViewport(0, 0, termWidth, gridHeight);
        
        // Center on origin
        renderer_.centerViewport(0, 0);
    }
    
    void handleInput(InputEvent event, const InputState& state) {
        switch (event) {
            case InputEvent::StartPause:
                if (controller_.getState() == SimulationState::Running) {
                    controller_.pause();
                } else {
                    controller_.start();
                }
                needsRender_ = true;
                break;
                
            case InputEvent::Step:
                if (controller_.getState() != SimulationState::Running) {
                    controller_.step();
                }
                needsRender_ = true;
                break;
                
            case InputEvent::Reset:
                controller_.reset();
                needsRender_ = true;
                break;
                
            case InputEvent::Quit:
                running_ = false;
                break;
                
            case InputEvent::MoveUp:
            case InputEvent::MoveDown:
            case InputEvent::MoveLeft:
            case InputEvent::MoveRight:
                renderer_.moveViewport(state.viewportDeltaX, state.viewportDeltaY);
                needsRender_ = true;
                break;
                
            case InputEvent::CenterView:
                renderer_.centerViewport(0, 0);
                needsRender_ = true;
                break;
                
            case InputEvent::LoadPattern:
                promptLoadPattern();
                needsRender_ = true;
                break;
                
            case InputEvent::ToggleStats: {
                auto config = renderer_.getRenderConfig();
                config.showStats = state.statsVisible;
                renderer_.setRenderConfig(config);
                break;
            }
            
            case InputEvent::ToggleControls: {
                auto config = renderer_.getRenderConfig();
                config.showControls = state.controlsVisible;
                renderer_.setRenderConfig(config);
                break;
            }
            
            default:
                break;
        }
    }
    
    void promptLoadPattern() {
        // Pause simulation for pattern loading
        auto originalState = controller_.getState();
        controller_.pause();
        
        renderer_.clearScreen();
        std::cout << "Available patterns:\n";
        std::cout << "1. Glider\n";
        std::cout << "2. Blinker\n";
        std::cout << "3. Gosper Gun\n";
        std::cout << "Enter pattern number (1-3): ";
        
        char choice = input_.getChar();
        std::string patternFile;
        
        switch (choice) {
            case '1': patternFile = "config/glider.json"; break;
            case '2': patternFile = "config/blinker.json"; break;
            case '3': patternFile = "config/gosper_gun.json"; break;
            default:
                std::cout << "\nInvalid choice. Continuing...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                return;
        }
        
        try {
            controller_.setDefaultPattern(patternFile);
            controller_.loadPattern(patternFile);
            std::cout << "\nPattern loaded successfully!\n";
        } catch (const std::exception& e) {
            std::cout << "\nError loading pattern: " << e.what() << "\n";
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        // Restore simulation state
        if (originalState == SimulationState::Running) {
            controller_.start();
        }
    }
    
    void cleanup() {
        renderer_.clearScreen();
        std::cout << "Game of Life Console Application terminated.\n";
        std::cout << "Final stats:\n";
        std::cout << "Generation: " << controller_.getStats().generation << "\n";
        std::cout << "Living cells: " << controller_.getStats().livingCells << "\n";
    }
    
    GameConfig loadDefaultConfig() {
        GameConfig config;
        
        try {
            config.loadFromFile("config/default.json");
        } catch (const std::exception& e) {
            std::cout << "Could not load config file, using defaults: " << e.what() << "\n";
        }
        
        return config;
    }
    
    RenderConfig createRenderConfig() {
        RenderConfig config;
        config.aliveChar = '#';
        config.deadChar = ' ';
        config.borderChar = '.';
        config.showBorder = false; // Disable for better performance
        config.showStats = true;
        config.showControls = true;
        config.clearScreen = true;
        
        return config;
    }
};

int main() {
    try {
        ConsoleApplication app;
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}