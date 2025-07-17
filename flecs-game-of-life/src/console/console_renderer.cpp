#include <flecs_gol/console_renderer.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

namespace flecs_gol {

ConsoleRenderer::ConsoleRenderer(const ViewportConfig& config)
    : config_(config)
    , lastRenderTime_(std::chrono::high_resolution_clock::now()) {
    
    // Initialize terminal
    initializeBuffer();
    
#ifdef _WIN32
    // Enable ANSI color codes on Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
    
    // Get terminal size
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        terminalWidth_ = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        terminalHeight_ = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        terminalWidth_ = w.ws_col;
        terminalHeight_ = w.ws_row;
    }
#endif
    
    // Adjust viewport to terminal size if needed
    config_.width = std::min(config_.width, terminalWidth_ - 2);
    config_.height = std::min(config_.height, terminalHeight_ - 5); // Leave space for UI
    
    hideCursor();
    terminalInitialized_ = true;
}

void ConsoleRenderer::render(const SimulationController& controller) {
    auto renderStart = std::chrono::high_resolution_clock::now();
    
    // Get current state and cells
    auto state = controller.getState();
    
    // Calculate viewport bounds
    auto [minX, minY] = calculateBounds();
    int32_t maxX = minX + static_cast<int32_t>(config_.width) - 1;
    int32_t maxY = minY + static_cast<int32_t>(config_.height) - 1;
    
    // Get cells in viewport
    auto cells = controller.getCellsInRegion(minX, maxX, minY, maxY);
    
    // Auto-center on activity if enabled
    if (config_.autoCenter && !cells.empty()) {
        auto [activityX, activityY] = findActivityCenter(cells);
        config_.centerX = activityX;
        config_.centerY = activityY;
    }
    
    // Clear and render
    clearScreen();
    
    if (config_.showBorder) {
        renderBorder();
    }
    
    renderGrid(cells);
    
    if (config_.showUI) {
        renderUI(state);
    }
    
    flushBuffer();
    
    // Update render FPS
    auto renderEnd = std::chrono::high_resolution_clock::now();
    auto renderDuration = std::chrono::duration_cast<std::chrono::milliseconds>(renderEnd - renderStart);
    
    if (renderDuration.count() > 0) {
        renderFPS_ = 1000.0 / renderDuration.count();
    }
    
    lastRenderTime_ = renderStart;
}

void ConsoleRenderer::setViewport(int32_t centerX, int32_t centerY) {
    config_.centerX = centerX;
    config_.centerY = centerY;
    config_.autoCenter = false;
}

void ConsoleRenderer::moveViewport(int32_t deltaX, int32_t deltaY) {
    config_.centerX += deltaX;
    config_.centerY += deltaY;
    config_.autoCenter = false;
}

void ConsoleRenderer::zoomIn() {
    if (config_.width > 20 && config_.height > 10) {
        config_.width = static_cast<uint32_t>(config_.width * 0.8);
        config_.height = static_cast<uint32_t>(config_.height * 0.8);
    }
}

void ConsoleRenderer::zoomOut() {
    if (config_.width < terminalWidth_ - 2 && config_.height < terminalHeight_ - 5) {
        config_.width = static_cast<uint32_t>(config_.width * 1.25);
        config_.height = static_cast<uint32_t>(config_.height * 1.25);
        
        // Clamp to terminal size
        config_.width = std::min(config_.width, terminalWidth_ - 2);
        config_.height = std::min(config_.height, terminalHeight_ - 5);
    }
}

void ConsoleRenderer::resetViewport() {
    config_.centerX = 0;
    config_.centerY = 0;
    config_.width = std::min(80u, terminalWidth_ - 2);
    config_.height = std::min(24u, terminalHeight_ - 5);
    config_.autoCenter = true;
}

void ConsoleRenderer::setAutoCenter(bool enabled) {
    config_.autoCenter = enabled;
}

void ConsoleRenderer::setRenderChars(char live, char dead, char border) {
    config_.liveChar = live;
    config_.deadChar = dead;
    config_.borderChar = border;
}

void ConsoleRenderer::setColors(bool enabled) {
    config_.useColors = enabled;
}

void ConsoleRenderer::setShowUI(bool enabled) {
    config_.showUI = enabled;
}

void ConsoleRenderer::setShowBorder(bool enabled) {
    config_.showBorder = enabled;
}

std::pair<int32_t, int32_t> ConsoleRenderer::getViewportCenter() const {
    return {config_.centerX, config_.centerY};
}

std::pair<uint32_t, uint32_t> ConsoleRenderer::getViewportSize() const {
    return {config_.width, config_.height};
}

void ConsoleRenderer::clearScreen() {
    if (bufferInitialized_) {
        for (auto& row : screenBuffer_) {
            std::fill(row.begin(), row.end(), ' ');
        }
    } else {
        std::cout << CLEAR_SCREEN << CURSOR_HOME;
    }
}

void ConsoleRenderer::renderGrid(const std::vector<CellData>& cells) {
    // First, fill grid area with dead chars
    auto [minX, minY] = calculateBounds();
    
    for (uint32_t screenY = 1; screenY <= config_.height; ++screenY) {
        for (uint32_t screenX = 1; screenX <= config_.width; ++screenX) {
            writeToBuffer(screenX, screenY, config_.deadChar);
        }
    }
    
    // Then draw live cells
    if (config_.useColors) {
        setColor(COLOR_GREEN);
    }
    
    for (const auto& cell : cells) {
        if (isInViewport(cell.x, cell.y)) {
            auto [screenX, screenY] = worldToScreen(cell.x, cell.y);
            writeToBuffer(screenX, screenY, config_.liveChar);
        }
    }
    
    if (config_.useColors) {
        resetColor();
    }
}

void ConsoleRenderer::renderBorder() {
    if (config_.useColors) {
        setColor(COLOR_BLUE);
    }
    
    // Top and bottom borders
    for (uint32_t x = 0; x <= config_.width + 1; ++x) {
        writeToBuffer(x, 0, config_.horizontalBorder);
        writeToBuffer(x, config_.height + 1, config_.horizontalBorder);
    }
    
    // Left and right borders
    for (uint32_t y = 0; y <= config_.height + 1; ++y) {
        writeToBuffer(0, y, config_.verticalBorder);
        writeToBuffer(config_.width + 1, y, config_.verticalBorder);
    }
    
    // Corners
    writeToBuffer(0, 0, config_.borderChar);
    writeToBuffer(config_.width + 1, 0, config_.borderChar);
    writeToBuffer(0, config_.height + 1, config_.borderChar);
    writeToBuffer(config_.width + 1, config_.height + 1, config_.borderChar);
    
    if (config_.useColors) {
        resetColor();
    }
}

void ConsoleRenderer::renderUI(const SimulationState& state) {
    uint32_t uiStartY = config_.height + 3;
    
    if (config_.useColors) {
        setColor(COLOR_CYAN);
    }
    
    // First line: Generation and cell count
    std::ostringstream line1;
    line1 << "Generation: " << state.generation 
          << " | Cells: " << state.liveCellCount
          << " | Memory: " << (state.memoryUsage / 1024) << "KB";
    writeToBuffer(0, uiStartY, line1.str());
    
    // Second line: FPS and timing
    std::ostringstream line2;
    line2 << "FPS: " << std::fixed << std::setprecision(1) << state.actualFPS
          << " | Step: " << (state.lastStepTimeMicros / 1000.0) << "ms"
          << " | Avg: " << (state.averageStepTimeMicros / 1000.0) << "ms";
    writeToBuffer(0, uiStartY + 1, line2.str());
    
    // Third line: Status and controls
    std::ostringstream line3;
    if (state.isRunning) {
        if (state.isPaused) {
            line3 << "Status: PAUSED";
        } else {
            line3 << "Status: RUNNING";
        }
    } else {
        line3 << "Status: STOPPED";
    }
    
    line3 << " | Viewport: (" << config_.centerX << "," << config_.centerY << ")";
    writeToBuffer(0, uiStartY + 2, line3.str());
    
    // Controls help
    if (config_.useColors) {
        setColor(COLOR_YELLOW);
    }
    
    writeToBuffer(0, uiStartY + 4, "Controls: SPACE=pause/resume, .>=step, R=reset, Q=quit, WASD=move, +/-=zoom");
    
    if (config_.useColors) {
        resetColor();
    }
}

std::pair<int32_t, int32_t> ConsoleRenderer::calculateBounds() const {
    int32_t minX = config_.centerX - static_cast<int32_t>(config_.width / 2);
    int32_t minY = config_.centerY - static_cast<int32_t>(config_.height / 2);
    return {minX, minY};
}

std::pair<int32_t, int32_t> ConsoleRenderer::findActivityCenter(const std::vector<CellData>& cells) const {
    if (cells.empty()) {
        return {config_.centerX, config_.centerY};
    }
    
    int64_t sumX = 0, sumY = 0;
    for (const auto& cell : cells) {
        sumX += cell.x;
        sumY += cell.y;
    }
    
    return {
        static_cast<int32_t>(sumX / static_cast<int64_t>(cells.size())),
        static_cast<int32_t>(sumY / static_cast<int64_t>(cells.size()))
    };
}

bool ConsoleRenderer::isInViewport(int32_t x, int32_t y) const {
    auto [minX, minY] = calculateBounds();
    int32_t maxX = minX + static_cast<int32_t>(config_.width) - 1;
    int32_t maxY = minY + static_cast<int32_t>(config_.height) - 1;
    
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

std::pair<uint32_t, uint32_t> ConsoleRenderer::worldToScreen(int32_t worldX, int32_t worldY) const {
    auto [minX, minY] = calculateBounds();
    
    uint32_t screenX = static_cast<uint32_t>(worldX - minX) + 1; // +1 for border
    uint32_t screenY = static_cast<uint32_t>(worldY - minY) + 1; // +1 for border
    
    return {screenX, screenY};
}

std::pair<int32_t, int32_t> ConsoleRenderer::screenToWorld(uint32_t screenX, uint32_t screenY) const {
    auto [minX, minY] = calculateBounds();
    
    int32_t worldX = static_cast<int32_t>(screenX - 1) + minX; // -1 for border
    int32_t worldY = static_cast<int32_t>(screenY - 1) + minY; // -1 for border
    
    return {worldX, worldY};
}

void ConsoleRenderer::initializeBuffer() {
    screenBuffer_.resize(terminalHeight_);
    for (auto& row : screenBuffer_) {
        row.resize(terminalWidth_, ' ');
    }
    bufferInitialized_ = true;
}

void ConsoleRenderer::writeToBuffer(uint32_t x, uint32_t y, char ch) {
    if (bufferInitialized_ && y < screenBuffer_.size() && x < screenBuffer_[y].size()) {
        screenBuffer_[y][x] = ch;
    } else {
        // Fallback to direct output
        moveCursor(x, y);
        std::cout << ch;
    }
}

void ConsoleRenderer::writeToBuffer(uint32_t x, uint32_t y, const std::string& str) {
    if (bufferInitialized_ && y < screenBuffer_.size()) {
        for (size_t i = 0; i < str.length() && (x + i) < screenBuffer_[y].size(); ++i) {
            screenBuffer_[y][x + i] = str[i];
        }
    } else {
        // Fallback to direct output
        moveCursor(x, y);
        std::cout << str;
    }
}

void ConsoleRenderer::flushBuffer() {
    if (bufferInitialized_) {
        std::cout << CURSOR_HOME;
        for (const auto& row : screenBuffer_) {
            for (char ch : row) {
                std::cout << ch;
            }
            std::cout << '\n';
        }
        std::cout << std::flush;
    }
}

void ConsoleRenderer::moveCursor(uint32_t x, uint32_t y) {
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
}

void ConsoleRenderer::setColor(const std::string& color) {
    if (config_.useColors) {
        std::cout << color;
    }
}

void ConsoleRenderer::resetColor() {
    if (config_.useColors) {
        std::cout << COLOR_RESET;
    }
}

void ConsoleRenderer::hideCursor() {
    std::cout << HIDE_CURSOR;
}

void ConsoleRenderer::showCursor() {
    std::cout << SHOW_CURSOR;
}

} // namespace flecs_gol