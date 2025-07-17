#pragma once

#include <flecs_gol/simulation_controller.h>
#include <string>
#include <vector>

namespace flecs_gol {

// Viewport configuration for console rendering
struct ViewportConfig {
    int32_t centerX = 0;
    int32_t centerY = 0;
    uint32_t width = 80;
    uint32_t height = 24;
    bool autoCenter = false; // Auto-center on cell activity
    bool showBorder = true;
    bool showUI = true;
    
    // Characters for rendering
    char liveChar = '#';
    char deadChar = ' ';
    char borderChar = '+';
    char horizontalBorder = '-';
    char verticalBorder = '|';
    
    // Color support (if terminal supports it)
    bool useColors = true;
};

class ConsoleRenderer {
public:
    explicit ConsoleRenderer(const ViewportConfig& config = ViewportConfig{});
    ~ConsoleRenderer() = default;
    
    // Main rendering method - takes controller as const reference
    void render(const SimulationController& controller);
    
    // Viewport control
    void setViewport(int32_t centerX, int32_t centerY);
    void moveViewport(int32_t deltaX, int32_t deltaY);
    void zoomIn();  // Decrease viewport size
    void zoomOut(); // Increase viewport size
    void resetViewport();
    void setAutoCenter(bool enabled);
    
    // Rendering configuration
    void setRenderChars(char live, char dead, char border = '+');
    void setColors(bool enabled);
    void setShowUI(bool enabled);
    void setShowBorder(bool enabled);
    
    // Viewport queries
    ViewportConfig getViewportConfig() const { return config_; }
    std::pair<int32_t, int32_t> getViewportCenter() const;
    std::pair<uint32_t, uint32_t> getViewportSize() const;
    
    // Clear screen utility
    void clearScreen();
    
private:
    // Internal rendering methods
    void renderGrid(const std::vector<CellData>& cells);
    void renderBorder();
    void renderUI(const SimulationState& state);
    void renderHelp();
    
    // Viewport calculations
    std::pair<int32_t, int32_t> calculateBounds() const;
    std::pair<int32_t, int32_t> findActivityCenter(const std::vector<CellData>& cells) const;
    bool isInViewport(int32_t x, int32_t y) const;
    std::pair<uint32_t, uint32_t> worldToScreen(int32_t worldX, int32_t worldY) const;
    std::pair<int32_t, int32_t> screenToWorld(uint32_t screenX, uint32_t screenY) const;
    
    // Terminal control
    void moveCursor(uint32_t x, uint32_t y);
    void setColor(const std::string& color);
    void resetColor();
    void hideCursor();
    void showCursor();
    
    // Screen buffer for flicker-free rendering
    void initializeBuffer();
    void writeToBuffer(uint32_t x, uint32_t y, char ch);
    void writeToBuffer(uint32_t x, uint32_t y, const std::string& str);
    void flushBuffer();
    
    ViewportConfig config_;
    
    // Screen buffer
    std::vector<std::vector<char>> screenBuffer_;
    bool bufferInitialized_ = false;
    
    // Terminal state
    bool terminalInitialized_ = false;
    uint32_t terminalWidth_ = 80;
    uint32_t terminalHeight_ = 24;
    
    // Performance tracking for rendering
    std::chrono::high_resolution_clock::time_point lastRenderTime_;
    double renderFPS_ = 0.0;
    
    // Color codes for terminal output
    static constexpr const char* COLOR_RESET = "\033[0m";
    static constexpr const char* COLOR_GREEN = "\033[32m";
    static constexpr const char* COLOR_BRIGHT_GREEN = "\033[92m";
    static constexpr const char* COLOR_RED = "\033[31m";
    static constexpr const char* COLOR_YELLOW = "\033[33m";
    static constexpr const char* COLOR_BLUE = "\033[34m";
    static constexpr const char* COLOR_MAGENTA = "\033[35m";
    static constexpr const char* COLOR_CYAN = "\033[36m";
    static constexpr const char* COLOR_WHITE = "\033[37m";
    
    // Terminal control sequences
    static constexpr const char* CLEAR_SCREEN = "\033[2J";
    static constexpr const char* CURSOR_HOME = "\033[H";
    static constexpr const char* HIDE_CURSOR = "\033[?25l";
    static constexpr const char* SHOW_CURSOR = "\033[?25h";
};

} // namespace flecs_gol