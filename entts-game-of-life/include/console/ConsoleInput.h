#pragma once

#include <functional>
#include <unordered_map>
#include <cstdint>
#include <termios.h>

enum class InputEvent {
    StartPause,
    Step,
    Reset,
    Quit,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    ZoomIn,
    ZoomOut,
    CenterView,
    LoadPattern,
    SavePattern,
    ToggleStats,
    ToggleControls,
    Unknown
};

struct InputState {
    bool running{true};
    std::int32_t viewportDeltaX{0};
    std::int32_t viewportDeltaY{0};
    bool statsVisible{true};
    bool controlsVisible{true};
};

class ConsoleInput {
public:
    using InputHandler = std::function<void(InputEvent, const InputState&)>;
    
    ConsoleInput();
    ~ConsoleInput();
    
    // Input handling
    void pollInput();
    bool hasInput() const;
    InputEvent getNextEvent();
    
    // Event subscription
    void setInputHandler(InputHandler handler);
    
    // State management
    const InputState& getState() const { return state_; }
    void resetState();
    
    // Configuration
    void setMoveSpeed(std::int32_t speed) { moveSpeed_ = speed; }
    std::int32_t getMoveSpeed() const { return moveSpeed_; }
    
    // Platform-specific input
    char getChar() const;
    bool kbhit() const;

private:
    InputHandler inputHandler_;
    InputState state_;
    std::int32_t moveSpeed_{5};
    
    // Input mapping
    std::unordered_map<char, InputEvent> keyMap_;
    
    // Helper methods
    void initializeKeyMap();
    void processKey(char key);
    void handleMovement(InputEvent event);
    void handleToggle(InputEvent event);
    
    // Platform-specific helpers
    void enableRawMode();
    void disableRawMode();
    
#ifdef _WIN32
    void* originalConsoleMode_;
#else
    struct termios originalTermios_;
    bool rawModeEnabled_{false};
#endif
};