#include <flecs_gol/console_input.h>
#include <iostream>
#include <cstdio>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

namespace flecs_gol {

ConsoleInput::ConsoleInput() {
    setupTerminal();
}

ConsoleInput::~ConsoleInput() {
    stop();
    restoreTerminal();
}

void ConsoleInput::start() {
    if (!inputRunning_) {
        shouldStop_ = false;
        inputRunning_ = true;
        inputThread_ = std::thread(&ConsoleInput::inputLoop, this);
    }
}

void ConsoleInput::stop() {
    shouldStop_ = true;
    inputRunning_ = false;
    
    if (inputThread_.joinable()) {
        inputThread_.join();
    }
}

void ConsoleInput::setInputCallback(InputCallback callback) {
    inputCallback_ = std::move(callback);
}

void ConsoleInput::setPositionCallback(PositionCallback callback) {
    positionCallback_ = std::move(callback);
}

void ConsoleInput::setNonBlocking(bool nonBlocking) {
    nonBlocking_ = nonBlocking;
}

void ConsoleInput::setMovementSensitivity(int32_t sensitivity) {
    movementSensitivity_ = sensitivity;
}

bool ConsoleInput::hasInput() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return !inputQueue_.empty();
}

InputEvent ConsoleInput::getNextInput() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    if (inputQueue_.empty()) {
        return InputEvent::UNKNOWN;
    }
    
    InputEvent event = inputQueue_.front();
    inputQueue_.pop();
    return event;
}

std::string ConsoleInput::getKeyDescription(InputEvent event) {
    switch (event) {
        case InputEvent::PAUSE_RESUME: return "SPACE - Pause/Resume simulation";
        case InputEvent::STEP: return ". or > - Single step";
        case InputEvent::RESET: return "R - Reset simulation";
        case InputEvent::QUIT: return "Q/ESC - Quit";
        case InputEvent::MOVE_UP: return "W - Move view up";
        case InputEvent::MOVE_DOWN: return "S - Move view down";
        case InputEvent::MOVE_LEFT: return "A - Move view left";
        case InputEvent::MOVE_RIGHT: return "D - Move view right";
        case InputEvent::ZOOM_IN: return "+ - Zoom in";
        case InputEvent::ZOOM_OUT: return "- - Zoom out";
        case InputEvent::CENTER_VIEW: return "C - Center view";
        case InputEvent::TOGGLE_AUTO_CENTER: return "T - Toggle auto-center";
        case InputEvent::LOAD_GLIDER: return "1 - Load glider pattern";
        case InputEvent::LOAD_BLINKER: return "2 - Load blinker pattern";
        case InputEvent::LOAD_GOSPER_GUN: return "3 - Load Gosper gun pattern";
        case InputEvent::CLEAR_GRID: return "X - Clear grid";
        case InputEvent::TOGGLE_UI: return "U - Toggle UI display";
        case InputEvent::TOGGLE_BORDER: return "B - Toggle border";
        case InputEvent::TOGGLE_COLORS: return "L - Toggle colors";
        case InputEvent::HELP: return "H - Show help";
        case InputEvent::SPEED_UP: return "F - Speed up";
        case InputEvent::SLOW_DOWN: return "G - Slow down";
        case InputEvent::TOGGLE_EDIT_MODE: return "E - Toggle edit mode";
        case InputEvent::PLACE_CELL: return "ENTER - Place/remove cell";
        default: return "Unknown key";
    }
}

std::string ConsoleInput::getHelpText() {
    return R"(
=== Conway's Game of Life - Controls ===

Simulation Control:
  SPACE    - Pause/Resume simulation
  . or >   - Single step (when paused)
  R        - Reset simulation
  Q/ESC    - Quit application

View Control:
  W,A,S,D  - Move viewport
  +/-      - Zoom in/out
  C        - Center view on origin
  T        - Toggle auto-center on activity

Pattern Loading:
  1        - Load glider pattern
  2        - Load blinker pattern  
  3        - Load Gosper gun pattern
  X        - Clear all cells

Display Options:
  U        - Toggle UI information
  B        - Toggle viewport border
  L        - Toggle colors
  H        - Show this help

Speed Control:
  F        - Increase simulation speed
  G        - Decrease simulation speed

Edit Mode:
  E        - Toggle manual cell editing
  ENTER    - Place/remove cell at center

Press any key to continue...
)";
}

void ConsoleInput::inputLoop() {
    while (!shouldStop_) {
        int key = getKeyPress();
        
        if (key != -1) {
            processInput(key);
        }
        
        if (nonBlocking_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

int ConsoleInput::getKeyPress() {
#ifdef _WIN32
    if (_kbhit()) {
        int key = _getch();
        
        // Handle special keys (arrow keys, function keys)
        if (key == 0 || key == 224) {
            key = _getch(); // Get the actual key code
            switch (key) {
                case 72: return KEY_UP;
                case 80: return KEY_DOWN;
                case 75: return KEY_LEFT;
                case 77: return KEY_RIGHT;
                default: return -1;
            }
        }
        
        return key;
    }
    return -1;
#else
    if (nonBlocking_) {
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10ms timeout
        
        int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);
        
        if (result <= 0) {
            return -1; // No input available or error
        }
    }
    
    int key = getchar();
    
    // Handle escape sequences (arrow keys)
    if (key == 27) { // ESC
        // Set a short timeout for escape sequences
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms timeout
        
        int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);
        if (result > 0) {
            int next1 = getchar();
            if (next1 == '[') {
                int next2 = getchar();
                switch (next2) {
                    case 'A': return KEY_UP;
                    case 'B': return KEY_DOWN;
                    case 'C': return KEY_RIGHT;
                    case 'D': return KEY_LEFT;
                    default: return KEY_ESCAPE;
                }
            } else {
                // Put back the character and return ESC
                ungetc(next1, stdin);
                return KEY_ESCAPE;
            }
        } else {
            return KEY_ESCAPE; // Timeout, treat as ESC
        }
    }
    
    return key;
#endif
}

void ConsoleInput::setupTerminal() {
#ifdef _WIN32
    // Windows console setup
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    originalConsoleMode_ = reinterpret_cast<void*>(mode);
    
    // Disable line input and echo
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hStdin, mode);
#else
    // Unix terminal setup
    if (isatty(STDIN_FILENO)) {
        tcgetattr(STDIN_FILENO, &originalTermios_);
        terminalConfigured_ = true;
        
        struct termios newTermios = originalTermios_;
        
        // Disable canonical mode and echo
        newTermios.c_lflag &= ~(ICANON | ECHO);
        newTermios.c_cc[VMIN] = 0;  // Non-blocking read
        newTermios.c_cc[VTIME] = 0;
        
        tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
        
        // Set non-blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }
#endif
}

void ConsoleInput::restoreTerminal() {
#ifdef _WIN32
    if (originalConsoleMode_) {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(hStdin, reinterpret_cast<DWORD>(reinterpret_cast<uintptr_t>(originalConsoleMode_)));
    }
#else
    if (terminalConfigured_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
        
        // Restore blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
}

InputEvent ConsoleInput::mapKeyToEvent(int key) {
    // Convert to lowercase for case-insensitive matching
    if (key >= 'A' && key <= 'Z') {
        key = key - 'A' + 'a';
    }
    
    switch (key) {
        case KEY_SPACE: return InputEvent::PAUSE_RESUME;
        case '.':
        case '>': return InputEvent::STEP;
        case 'r': return InputEvent::RESET;
        case 'q':
        case KEY_ESCAPE: return InputEvent::QUIT;
        
        case 'w':
        case KEY_UP: return InputEvent::MOVE_UP;
        case 's':
        case KEY_DOWN: return InputEvent::MOVE_DOWN;
        case 'a':
        case KEY_LEFT: return InputEvent::MOVE_LEFT;
        case 'd':
        case KEY_RIGHT: return InputEvent::MOVE_RIGHT;
        
        case '+':
        case '=': return InputEvent::ZOOM_IN;
        case '-':
        case '_': return InputEvent::ZOOM_OUT;
        
        case 'c': return InputEvent::CENTER_VIEW;
        case 't': return InputEvent::TOGGLE_AUTO_CENTER;
        
        case '1': return InputEvent::LOAD_GLIDER;
        case '2': return InputEvent::LOAD_BLINKER;
        case '3': return InputEvent::LOAD_GOSPER_GUN;
        case 'x': return InputEvent::CLEAR_GRID;
        
        case 'u': return InputEvent::TOGGLE_UI;
        case 'b': return InputEvent::TOGGLE_BORDER;
        case 'l': return InputEvent::TOGGLE_COLORS;
        case 'h': return InputEvent::HELP;
        
        case 'f': return InputEvent::SPEED_UP;
        case 'g': return InputEvent::SLOW_DOWN;
        
        case 'e': return InputEvent::TOGGLE_EDIT_MODE;
        case KEY_ENTER: return InputEvent::PLACE_CELL;
        
        default: return InputEvent::UNKNOWN;
    }
}

void ConsoleInput::processInput(int key) {
    InputEvent event = mapKeyToEvent(key);
    
    if (event != InputEvent::UNKNOWN) {
        addInputEvent(event);
        
        // Handle movement events specially
        if (positionCallback_) {
            int32_t deltaX = 0, deltaY = 0;
            
            switch (event) {
                case InputEvent::MOVE_LEFT:
                    deltaX = movementSensitivity_;
                    break;
                case InputEvent::MOVE_RIGHT:
                    deltaX = -movementSensitivity_;
                    break;
                case InputEvent::MOVE_UP:
                    deltaY = movementSensitivity_;
                    break;
                case InputEvent::MOVE_DOWN:
                    deltaY = -movementSensitivity_;
                    break;
                default:
                    break;
            }
            
            if (deltaX != 0 || deltaY != 0) {
                positionCallback_(deltaX, deltaY);
            }
        }
        
        // Call general input callback
        if (inputCallback_) {
            inputCallback_(event);
        }
    }
}

void ConsoleInput::addInputEvent(InputEvent event) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    inputQueue_.push(event);
}

} // namespace flecs_gol