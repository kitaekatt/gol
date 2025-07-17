#pragma once

#include "console/SimulationController.h"
#include <string>
#include <vector>
#include <cstdint>

struct RenderConfig {
    char aliveChar{'#'};
    char deadChar{' '};
    char borderChar{'.'};
    bool showBorder{true};
    bool showStats{true};
    bool showControls{true};
    bool clearScreen{true};
    std::int32_t viewportX{0};
    std::int32_t viewportY{0};
    std::int32_t viewportWidth{80};
    std::int32_t viewportHeight{24};
};

class ConsoleRenderer {
public:
    explicit ConsoleRenderer(const RenderConfig& config = RenderConfig{});
    
    // Core rendering interface
    void render(const SimulationController& controller);
    void renderFrame(const SimulationController& controller, 
                    std::int32_t startX, std::int32_t startY,
                    std::int32_t width, std::int32_t height);
    
    // Configuration
    void setRenderConfig(const RenderConfig& config) { config_ = config; }
    const RenderConfig& getRenderConfig() const { return config_; }
    
    // Viewport control
    void setViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height);
    void centerViewport(std::int32_t centerX, std::int32_t centerY);
    void moveViewport(std::int32_t deltaX, std::int32_t deltaY);
    
    // Utility methods
    void clearScreen();
    void moveCursor(std::int32_t x, std::int32_t y);
    std::pair<std::int32_t, std::int32_t> getTerminalSize();

private:
    RenderConfig config_;
    
    // Rendering helpers
    void renderGrid(const SimulationController& controller);
    void renderStats(const SimulationStats& stats);
    void renderControls();
    void renderBorder(std::int32_t width, std::int32_t height);
    
    // String formatting helpers
    std::string formatStats(const SimulationStats& stats);
    std::string formatState(SimulationState state);
    std::string repeatChar(char c, std::size_t count);
};