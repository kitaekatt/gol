#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

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

// Input event types
enum class InputEvent {
    // Simulation control
    PAUSE_RESUME,      // Space bar
    STEP,              // S key
    RESET,             // R key
    QUIT,              // Q key or Escape
    
    // View control
    MOVE_UP,           // W key
    MOVE_DOWN,         // S key  
    MOVE_LEFT,         // A key
    MOVE_RIGHT,        // D key
    ZOOM_IN,           // + key
    ZOOM_OUT,          // - key
    CENTER_VIEW,       // C key
    TOGGLE_AUTO_CENTER,// T key
    
    // Pattern loading
    LOAD_GLIDER,       // 1 key
    LOAD_BLINKER,      // 2 key
    LOAD_GOSPER_GUN,   // 3 key
    CLEAR_GRID,        // X key
    
    // Display options
    TOGGLE_UI,         // U key
    TOGGLE_BORDER,     // B key
    TOGGLE_COLORS,     // L key (coLors)
    HELP,              // H key
    
    // Speed control
    SPEED_UP,          // F key (faster)
    SLOW_DOWN,         // G key (slower)
    
    // Manual cell editing
    TOGGLE_EDIT_MODE,  // E key
    PLACE_CELL,        // Enter key in edit mode
    
    UNKNOWN
};

// Callback types for input handling
using InputCallback = std::function<void(InputEvent event)>;
using PositionCallback = std::function<void(int32_t deltaX, int32_t deltaY)>;

class ConsoleInput {
public:
    ConsoleInput();
    ~ConsoleInput();
    
    // Input handling control
    void start();
    void stop();
    bool isRunning() const { return inputRunning_; }
    
    // Callback registration
    void setInputCallback(InputCallback callback);
    void setPositionCallback(PositionCallback callback);
    
    // Configuration
    void setNonBlocking(bool nonBlocking);
    void setMovementSensitivity(int32_t sensitivity);
    
    // Manual input polling (alternative to callbacks)
    bool hasInput() const;
    InputEvent getNextInput();
    
    // Input state queries
    bool isEditModeEnabled() const { return editMode_; }
    void setEditMode(bool enabled) { editMode_ = enabled; }
    
    // Key mapping utilities
    static std::string getKeyDescription(InputEvent event);
    static std::string getHelpText();

private:
    // Platform-specific input methods
    void inputLoop();
    int getKeyPress();
    void setupTerminal();
    void restoreTerminal();
    
    // Input processing
    InputEvent mapKeyToEvent(int key);
    void processInput(int key);
    void addInputEvent(InputEvent event);
    
    // Thread management
    std::thread inputThread_;
    std::atomic<bool> inputRunning_{false};
    std::atomic<bool> shouldStop_{false};
    
    // Event queue for thread-safe input handling
    std::queue<InputEvent> inputQueue_;
    mutable std::mutex queueMutex_;
    
    // Callbacks
    InputCallback inputCallback_;
    PositionCallback positionCallback_;
    
    // Configuration
    bool nonBlocking_ = true;
    int32_t movementSensitivity_ = 1;
    bool editMode_ = false;
    
    // Platform-specific terminal state
#ifdef _WIN32
    void* originalConsoleMode_ = nullptr;
#else
    struct termios originalTermios_;
    bool terminalConfigured_ = false;
#endif
    
    // Key mapping constants
    static constexpr int KEY_ESCAPE = 27;
    static constexpr int KEY_SPACE = 32;
    static constexpr int KEY_ENTER = 13;
    
    // Arrow keys (platform-specific sequences)
#ifdef _WIN32
    static constexpr int KEY_UP = 72;
    static constexpr int KEY_DOWN = 80;
    static constexpr int KEY_LEFT = 75;
    static constexpr int KEY_RIGHT = 77;
#else
    static constexpr int KEY_UP = 65;
    static constexpr int KEY_DOWN = 66;
    static constexpr int KEY_LEFT = 68;
    static constexpr int KEY_RIGHT = 67;
#endif
};

} // namespace flecs_gol