#include "console/ConsoleRenderer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

ConsoleRenderer::ConsoleRenderer(const RenderConfig& config) 
    : config_(config) {
}

void ConsoleRenderer::render(const SimulationController& controller) {
    if (config_.clearScreen) {
        clearScreen();
    }
    
    renderGrid(controller);
    
    if (config_.showStats) {
        renderStats(controller.getStats());
    }
    
    if (config_.showControls) {
        renderControls();
    }
    
    std::cout.flush();
}

void ConsoleRenderer::renderFrame(const SimulationController& controller,
                                 std::int32_t startX, std::int32_t startY,
                                 std::int32_t width, std::int32_t height) {
    
    // Render border if enabled
    if (config_.showBorder) {
        renderBorder(width + 2, height + 2);
        moveCursor(1, 1);
    }
    
    // Render grid content
    for (std::int32_t y = 0; y < height; ++y) {
        for (std::int32_t x = 0; x < width; ++x) {
            std::int32_t worldX = startX + x;
            std::int32_t worldY = startY + y;
            
            bool alive = controller.isCellAlive(worldX, worldY);
            char cellChar = alive ? config_.aliveChar : config_.deadChar;
            std::cout << cellChar;
        }
        
        if (y < height - 1) {
            std::cout << '\n';
            if (config_.showBorder) {
                std::cout << config_.borderChar;
            }
        }
    }
}

void ConsoleRenderer::setViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) {
    config_.viewportX = x;
    config_.viewportY = y;
    config_.viewportWidth = width;
    config_.viewportHeight = height;
}

void ConsoleRenderer::centerViewport(std::int32_t centerX, std::int32_t centerY) {
    config_.viewportX = centerX - config_.viewportWidth / 2;
    config_.viewportY = centerY - config_.viewportHeight / 2;
}

void ConsoleRenderer::moveViewport(std::int32_t deltaX, std::int32_t deltaY) {
    config_.viewportX += deltaX;
    config_.viewportY += deltaY;
}

void ConsoleRenderer::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleRenderer::moveCursor(std::int32_t x, std::int32_t y) {
#ifdef _WIN32
    COORD coord = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
#endif
}

std::pair<std::int32_t, std::int32_t> ConsoleRenderer::getTerminalSize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return {csbi.srWindow.Right - csbi.srWindow.Left + 1,
                csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
    }
    return {80, 24}; // Default fallback
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return {w.ws_col, w.ws_row};
    }
    return {80, 24}; // Default fallback
#endif
}

void ConsoleRenderer::renderGrid(const SimulationController& controller) {
    // Adjust viewport height for stats and controls
    std::int32_t gridHeight = config_.viewportHeight;
    if (config_.showStats) gridHeight -= 3;
    if (config_.showControls) gridHeight -= 3;
    
    renderFrame(controller, 
               config_.viewportX, config_.viewportY,
               config_.viewportWidth, gridHeight);
}

void ConsoleRenderer::renderStats(const SimulationStats& stats) {
    std::cout << "\n" << repeatChar('=', config_.viewportWidth) << "\n";
    std::cout << formatStats(stats) << "\n";
}

void ConsoleRenderer::renderControls() {
    std::cout << repeatChar('-', config_.viewportWidth) << "\n";
    std::cout << "Controls: [SPACE] Start/Pause | [>/.] Step | [R] Reset | [Q] Quit | [W/A/S/D] Move viewport | [L] Load Pattern\n";
}

void ConsoleRenderer::renderBorder(std::int32_t width, std::int32_t height) {
    // Top border
    std::cout << repeatChar(config_.borderChar, width) << "\n";
    
    // Side borders will be rendered by renderFrame
    
    // Bottom border will be rendered after grid content
}

std::string ConsoleRenderer::formatStats(const SimulationStats& stats) {
    std::ostringstream oss;
    oss << "Gen: " << std::setw(6) << stats.generation
        << " | Cells: " << std::setw(6) << stats.livingCells
        << " | FPS: " << std::fixed << std::setprecision(1) << stats.actualFps
        << " | Step: " << stats.lastStepTime.count() << "ms";
    
    if (stats.isStable) {
        oss << " | STABLE";
    }
    
    return oss.str();
}

std::string ConsoleRenderer::formatState(SimulationState state) {
    switch (state) {
        case SimulationState::Stopped: return "STOPPED";
        case SimulationState::Running: return "RUNNING";
        case SimulationState::Paused: return "PAUSED";
        case SimulationState::Stepping: return "STEPPING";
        default: return "UNKNOWN";
    }
}

std::string ConsoleRenderer::repeatChar(char c, std::size_t count) {
    return std::string(count, c);
}