use std::collections::HashMap;
use std::io;

#[cfg(windows)]
use winapi::um::conio::{_getch, _kbhit};

#[cfg(unix)]
extern crate libc;

/// Input events from the console - simplified like EntTS
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum InputEvent {
    // Simulation control
    StartPause,
    Step,
    Reset,
    Quit,
    
    // Movement
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    
    // View control
    ZoomIn,
    ZoomOut,
    CenterView,
    
    // Pattern loading
    LoadPattern,
    SavePattern,
    
    // Display toggles
    ToggleStats,
    ToggleControls,
    
    // Unknown input
    Unknown,
}

/// Input state tracking - like EntTS
#[derive(Debug, Clone)]
pub struct InputState {
    pub running: bool,
    pub viewport_delta_x: i32,
    pub viewport_delta_y: i32,
    pub stats_visible: bool,
    pub controls_visible: bool,
}

impl Default for InputState {
    fn default() -> Self {
        Self {
            running: true,
            viewport_delta_x: 0,
            viewport_delta_y: 0,
            stats_visible: true,
            controls_visible: true,
        }
    }
}

/// Direct console input handler - EntTS style
pub struct ConsoleInput {
    key_map: HashMap<char, InputEvent>,
    state: InputState,
    move_speed: i32,
    
    #[cfg(unix)]
    original_termios: Option<libc::termios>,
    #[cfg(unix)]
    raw_mode_enabled: bool,
}

impl ConsoleInput {
    /// Create a new console input handler - EntTS style
    pub fn new() -> io::Result<Self> {
        let mut input = Self {
            key_map: HashMap::new(),
            state: InputState::default(),
            move_speed: 5,
            
            #[cfg(unix)]
            original_termios: None,
            #[cfg(unix)]
            raw_mode_enabled: false,
        };
        
        input.initialize_key_map();
        input.enable_raw_mode()?;
        
        Ok(input)
    }
    
    /// Poll for input without blocking - EntTS style
    pub fn poll_input(&mut self) -> Option<InputEvent> {
        if self.has_input() {
            let key = self.get_char();
            Some(self.process_key(key))
        } else {
            None
        }
    }
    
    /// Check if input is available - EntTS style
    pub fn has_input(&self) -> bool {
        #[cfg(windows)]
        {
            unsafe { _kbhit() != 0 }
        }
        
        #[cfg(unix)]
        {
            use libc::{fd_set, select, timeval, FD_SET, FD_ZERO, STDIN_FILENO};
            use std::mem::MaybeUninit;
            
            unsafe {
                let mut readfds: fd_set = MaybeUninit::zeroed().assume_init();
                FD_ZERO(&mut readfds);
                FD_SET(STDIN_FILENO, &mut readfds);
                
                let mut timeout = timeval { tv_sec: 0, tv_usec: 0 };
                
                select(STDIN_FILENO + 1, &mut readfds, std::ptr::null_mut(), std::ptr::null_mut(), &mut timeout) > 0
            }
        }
    }
    
    /// Get a character from input - EntTS style
    fn get_char(&self) -> char {
        #[cfg(windows)]
        {
            unsafe { _getch() as u8 as char }
        }
        
        #[cfg(unix)]
        {
            use std::io::Read;
            let mut buffer = [0u8; 1];
            std::io::stdin().read_exact(&mut buffer).unwrap_or(());
            buffer[0] as char
        }
    }
    
    /// Process a key press and update state - EntTS style
    fn process_key(&mut self, key: char) -> InputEvent {
        let event = self.key_map.get(&key.to_ascii_lowercase())
            .copied()
            .unwrap_or(InputEvent::Unknown);
        
        // Handle state changes
        match event {
            InputEvent::MoveUp | InputEvent::MoveDown | 
            InputEvent::MoveLeft | InputEvent::MoveRight => {
                self.handle_movement(event.clone());
            }
            InputEvent::ToggleStats => {
                self.state.stats_visible = !self.state.stats_visible;
            }
            InputEvent::ToggleControls => {
                self.state.controls_visible = !self.state.controls_visible;
            }
            InputEvent::Quit => {
                self.state.running = false;
            }
            _ => {}
        }
        
        event
    }
    
    /// Handle movement input - EntTS style
    fn handle_movement(&mut self, event: InputEvent) {
        self.state.viewport_delta_x = 0;
        self.state.viewport_delta_y = 0;
        
        match event {
            InputEvent::MoveUp => self.state.viewport_delta_y = -self.move_speed,
            InputEvent::MoveDown => self.state.viewport_delta_y = self.move_speed,
            InputEvent::MoveLeft => self.state.viewport_delta_x = -self.move_speed,
            InputEvent::MoveRight => self.state.viewport_delta_x = self.move_speed,
            _ => {}
        }
    }
    
    /// Initialize key mappings - EntTS style
    fn initialize_key_map(&mut self) {
        // Core controls
        self.key_map.insert(' ', InputEvent::StartPause);
        self.key_map.insert('r', InputEvent::Reset);
        self.key_map.insert('q', InputEvent::Quit);
        
        // Movement controls - WASD
        self.key_map.insert('w', InputEvent::MoveUp);
        self.key_map.insert('a', InputEvent::MoveLeft);
        self.key_map.insert('s', InputEvent::MoveDown);
        self.key_map.insert('d', InputEvent::MoveRight);
        
        // Step control
        self.key_map.insert('.', InputEvent::Step);
        self.key_map.insert('>', InputEvent::Step);
        
        // View controls
        self.key_map.insert('+', InputEvent::ZoomIn);
        self.key_map.insert('-', InputEvent::ZoomOut);
        self.key_map.insert('c', InputEvent::CenterView);
        
        // File operations
        self.key_map.insert('l', InputEvent::LoadPattern);
        self.key_map.insert('o', InputEvent::SavePattern);
        
        // Display toggles
        self.key_map.insert('i', InputEvent::ToggleStats);
        self.key_map.insert('h', InputEvent::ToggleControls);
    }
    
    /// Enable raw terminal mode - EntTS style
    fn enable_raw_mode(&mut self) -> io::Result<()> {
        #[cfg(windows)]
        {
            // Windows console mode handling would go here
            Ok(())
        }
        
        #[cfg(unix)]
        {
            use libc::{tcgetattr, tcsetattr, termios, ECHO, ICANON, ISIG, IXON, ICRNL, TCSAFLUSH, STDIN_FILENO, VMIN, VTIME};
            use std::mem::MaybeUninit;
            
            unsafe {
                let mut termios: termios = MaybeUninit::zeroed().assume_init();
                if tcgetattr(STDIN_FILENO, &mut termios) != 0 {
                    return Err(io::Error::last_os_error());
                }
                
                self.original_termios = Some(termios);
                
                let mut raw = termios;
                raw.c_lflag &= !(ECHO | ICANON | ISIG);
                raw.c_iflag &= !(IXON | ICRNL);
                raw.c_cc[VMIN] = 1;
                raw.c_cc[VTIME] = 1;
                
                if tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0 {
                    return Err(io::Error::last_os_error());
                }
                
                self.raw_mode_enabled = true;
            }
            
            Ok(())
        }
    }
    
    /// Disable raw terminal mode - EntTS style
    fn disable_raw_mode(&mut self) {
        #[cfg(unix)]
        {
            if self.raw_mode_enabled {
                if let Some(original) = self.original_termios {
                    unsafe {
                        libc::tcsetattr(libc::STDIN_FILENO, libc::TCSAFLUSH, &original);
                    }
                    self.raw_mode_enabled = false;
                }
            }
        }
    }
    
    /// Get current input state
    pub fn get_state(&self) -> &InputState {
        &self.state
    }
    
    /// Reset input state
    pub fn reset_state(&mut self) {
        self.state = InputState::default();
    }
    
    /// Set movement speed
    pub fn set_move_speed(&mut self, speed: i32) {
        self.move_speed = speed;
    }
}

impl Drop for ConsoleInput {
    fn drop(&mut self) {
        self.disable_raw_mode();
    }
}