#include "console/ConsoleInput.h"
#include <iostream>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#endif

ConsoleInput::ConsoleInput() {
    initializeKeyMap();
    enableRawMode();
}

ConsoleInput::~ConsoleInput() {
    disableRawMode();
}

void ConsoleInput::pollInput() {
    if (kbhit()) {
        char key = getChar();
        processKey(key);
    }
}

bool ConsoleInput::hasInput() const {
    return kbhit();
}

InputEvent ConsoleInput::getNextEvent() {
    if (hasInput()) {
        char key = getChar();
        auto it = keyMap_.find(std::tolower(key));
        return it != keyMap_.end() ? it->second : InputEvent::Unknown;
    }
    return InputEvent::Unknown;
}

void ConsoleInput::setInputHandler(InputHandler handler) {
    inputHandler_ = std::move(handler);
}

void ConsoleInput::resetState() {
    state_ = InputState{};
}

char ConsoleInput::getChar() const {
#ifdef _WIN32
    return _getch();
#else
    return getchar();
#endif
}

bool ConsoleInput::kbhit() const {
#ifdef _WIN32
    return _kbhit();
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    struct timeval timeout = {0, 0}; // No wait
    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0;
#endif
}

void ConsoleInput::initializeKeyMap() {
    // Core controls
    keyMap_[' '] = InputEvent::StartPause;
    keyMap_['r'] = InputEvent::Reset;
    keyMap_['q'] = InputEvent::Quit;
    
    // Movement controls - WASD  
    keyMap_['w'] = InputEvent::MoveUp;
    keyMap_['a'] = InputEvent::MoveLeft;
    keyMap_['s'] = InputEvent::MoveDown;
    keyMap_['d'] = InputEvent::MoveRight;
    
    // Step controls
    keyMap_['>'] = InputEvent::Step;     // Right arrow alternative
    keyMap_['.'] = InputEvent::Step;     // Period key (easier to type)
    
    // Arrow keys 
    keyMap_[72] = InputEvent::MoveUp;    // Up arrow (Windows)
    keyMap_[80] = InputEvent::MoveDown;  // Down arrow (Windows)
    keyMap_[75] = InputEvent::MoveLeft;  // Left arrow (Windows)
    keyMap_[77] = InputEvent::Step;      // Right arrow = Step (Windows)
    
    // View controls
    keyMap_['+'] = InputEvent::ZoomIn;
    keyMap_['-'] = InputEvent::ZoomOut;
    keyMap_['c'] = InputEvent::CenterView;
    
    // File operations
    keyMap_['l'] = InputEvent::LoadPattern;
    keyMap_['o'] = InputEvent::SavePattern;
    
    // Display toggles
    keyMap_['i'] = InputEvent::ToggleStats;
    keyMap_['h'] = InputEvent::ToggleControls;
}

void ConsoleInput::processKey(char key) {
    auto it = keyMap_.find(std::tolower(key));
    InputEvent event = it != keyMap_.end() ? it->second : InputEvent::Unknown;
    
    
    // Handle state changes
    switch (event) {
        case InputEvent::MoveUp:
        case InputEvent::MoveDown:
        case InputEvent::MoveLeft:
        case InputEvent::MoveRight:
            handleMovement(event);
            break;
            
        case InputEvent::ToggleStats:
        case InputEvent::ToggleControls:
            handleToggle(event);
            break;
            
        case InputEvent::Quit:
            // Let the application handle quit, don't set state_.running = false
            break;
            
        default:
            break;
    }
    
    // Notify handler
    if (inputHandler_) {
        inputHandler_(event, state_);
    }
}

void ConsoleInput::handleMovement(InputEvent event) {
    state_.viewportDeltaX = 0;
    state_.viewportDeltaY = 0;
    
    switch (event) {
        case InputEvent::MoveUp:
            state_.viewportDeltaY = -moveSpeed_;
            break;
        case InputEvent::MoveDown:
            state_.viewportDeltaY = moveSpeed_;
            break;
        case InputEvent::MoveLeft:
            state_.viewportDeltaX = -moveSpeed_;
            break;
        case InputEvent::MoveRight:
            state_.viewportDeltaX = moveSpeed_;
            break;
        default:
            break;
    }
}

void ConsoleInput::handleToggle(InputEvent event) {
    switch (event) {
        case InputEvent::ToggleStats:
            state_.statsVisible = !state_.statsVisible;
            break;
        case InputEvent::ToggleControls:
            state_.controlsVisible = !state_.controlsVisible;
            break;
        default:
            break;
    }
}

void ConsoleInput::enableRawMode() {
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    originalConsoleMode_ = reinterpret_cast<void*>(mode);
    SetConsoleMode(hInput, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
#else
    tcgetattr(STDIN_FILENO, &originalTermios_);
    struct termios raw = originalTermios_;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;  // Wait for at least 1 character
    raw.c_cc[VTIME] = 1; // Wait up to 0.1 seconds
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    rawModeEnabled_ = true;
#endif
}

void ConsoleInput::disableRawMode() {
#ifdef _WIN32
    if (originalConsoleMode_) {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(hInput, reinterpret_cast<DWORD>(originalConsoleMode_));
    }
#else
    if (rawModeEnabled_) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios_);
        rawModeEnabled_ = false;
    }
#endif
}