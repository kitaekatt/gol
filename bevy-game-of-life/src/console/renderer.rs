use super::controller::SimulationSnapshot;
use std::collections::HashSet;
use std::io::{self, Write};

/// Render configuration - EntTS style
#[derive(Debug, Clone)]
pub struct RenderConfig {
    pub alive_char: char,
    pub dead_char: char,
    pub border_char: char,
    pub show_border: bool,
    pub show_stats: bool,
    pub show_controls: bool,
    pub clear_screen: bool,
    pub viewport_x: i32,
    pub viewport_y: i32,
    pub viewport_width: i32,
    pub viewport_height: i32,
}

impl Default for RenderConfig {
    fn default() -> Self {
        Self {
            alive_char: '#',
            dead_char: ' ',
            border_char: '.',
            show_border: true,
            show_stats: true,
            show_controls: true,
            clear_screen: true,
            viewport_x: 0,
            viewport_y: 0,
            viewport_width: 80,
            viewport_height: 24,
        }
    }
}

/// Simplified console renderer - EntTS style
pub struct ConsoleRenderer {
    config: RenderConfig,
}

impl ConsoleRenderer {
    /// Create a new console renderer - EntTS style
    pub fn new(config: RenderConfig) -> Self {
        Self { config }
    }
    
    /// Create with default config
    pub fn default() -> Self {
        Self::new(RenderConfig::default())
    }
    
    /// Core rendering interface - EntTS style
    pub fn render(&self, snapshot: &SimulationSnapshot) -> Result<(), io::Error> {
        if self.config.clear_screen {
            self.clear_screen();
        }
        
        self.render_grid(snapshot);
        
        if self.config.show_stats {
            self.render_stats(snapshot);
        }
        
        if self.config.show_controls {
            self.render_controls();
        }
        
        io::stdout().flush()?;
        Ok(())
    }
    
    /// Render frame with specific bounds - EntTS style
    pub fn render_frame(&self, snapshot: &SimulationSnapshot, start_x: i32, start_y: i32, width: i32, height: i32) {
        let live_cells: HashSet<(i32, i32)> = snapshot.live_cells.iter().cloned().collect();
        
        // Render border if enabled
        if self.config.show_border {
            self.render_border(width + 2, height + 2);
            self.move_cursor(1, 1);
        }
        
        // Render grid content
        for y in 0..height {
            for x in 0..width {
                let world_x = start_x + x;
                let world_y = start_y + y;
                
                let alive = live_cells.contains(&(world_x, world_y));
                let cell_char = if alive { self.config.alive_char } else { self.config.dead_char };
                print!("{}", cell_char);
            }
            
            if y < height - 1 {
                print!("\n");
                if self.config.show_border {
                    print!("{}", self.config.border_char);
                }
            }
        }
    }
    
    /// Set viewport position and size - EntTS style
    pub fn set_viewport(&mut self, x: i32, y: i32, width: i32, height: i32) {
        self.config.viewport_x = x;
        self.config.viewport_y = y;
        self.config.viewport_width = width;
        self.config.viewport_height = height;
    }
    
    /// Center viewport on position - EntTS style
    pub fn center_viewport(&mut self, center_x: i32, center_y: i32) {
        self.config.viewport_x = center_x - self.config.viewport_width / 2;
        self.config.viewport_y = center_y - self.config.viewport_height / 2;
    }
    
    /// Move viewport by delta - EntTS style
    pub fn move_viewport(&mut self, delta_x: i32, delta_y: i32) {
        self.config.viewport_x += delta_x;
        self.config.viewport_y += delta_y;
    }
    
    /// Clear screen - EntTS style
    pub fn clear_screen(&self) {
        #[cfg(windows)]
        {
            std::process::Command::new("cls").status().ok();
        }
        
        #[cfg(unix)]
        {
            std::process::Command::new("clear").status().ok();
        }
    }
    
    /// Move cursor - EntTS style
    pub fn move_cursor(&self, x: i32, y: i32) {
        #[cfg(windows)]
        {
            use winapi::um::wincon::{SetConsoleCursorPosition, COORD};
            use winapi::um::processenv::GetStdHandle;
            use winapi::um::winbase::STD_OUTPUT_HANDLE;
            
            unsafe {
                let coord = COORD { X: x as i16, Y: y as i16 };
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
            }
        }
        
        #[cfg(unix)]
        {
            print!("\x1b[{};{}H", y + 1, x + 1);
        }
    }
    
    /// Get terminal size - EntTS style
    pub fn get_terminal_size(&self) -> (i32, i32) {
        #[cfg(windows)]
        {
            use winapi::um::wincon::{GetConsoleScreenBufferInfo, CONSOLE_SCREEN_BUFFER_INFO};
            use winapi::um::processenv::GetStdHandle;
            use winapi::um::winbase::STD_OUTPUT_HANDLE;
            
            unsafe {
                let mut csbi: CONSOLE_SCREEN_BUFFER_INFO = std::mem::zeroed();
                if GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &mut csbi) != 0 {
                    return (
                        (csbi.srWindow.Right - csbi.srWindow.Left + 1) as i32,
                        (csbi.srWindow.Bottom - csbi.srWindow.Top + 1) as i32,
                    );
                }
            }
        }
        
        #[cfg(unix)]
        {
            use libc::{ioctl, winsize, STDOUT_FILENO, TIOCGWINSZ};
            use std::mem::MaybeUninit;
            
            unsafe {
                let mut w: winsize = MaybeUninit::zeroed().assume_init();
                if ioctl(STDOUT_FILENO, TIOCGWINSZ, &mut w) == 0 {
                    return (w.ws_col as i32, w.ws_row as i32);
                }
            }
        }
        
        (80, 24) // Default fallback
    }
    
    /// Render grid - EntTS style
    fn render_grid(&self, snapshot: &SimulationSnapshot) {
        // Adjust viewport height for stats and controls
        let mut grid_height = self.config.viewport_height;
        if self.config.show_stats { grid_height -= 3; }
        if self.config.show_controls { grid_height -= 3; }
        
        self.render_frame(snapshot, 
                         self.config.viewport_x, self.config.viewport_y,
                         self.config.viewport_width, grid_height);
    }
    
    /// Render stats - EntTS style
    fn render_stats(&self, snapshot: &SimulationSnapshot) {
        println!("\n{}", self.repeat_char('=', self.config.viewport_width));
        println!("{}", self.format_stats(snapshot));
    }
    
    /// Render controls - EntTS style
    fn render_controls(&self) {
        println!("{}", self.repeat_char('-', self.config.viewport_width));
        println!("Controls: [SPACE] Start/Pause | [>/.] Step | [R] Reset | [Q] Quit | [W/A/S/D] Move | [L] Load Pattern");
    }
    
    /// Render border - EntTS style
    fn render_border(&self, width: i32, _height: i32) {
        // Top border
        println!("{}", self.repeat_char(self.config.border_char, width));
    }
    
    /// Format stats string - EntTS style
    fn format_stats(&self, snapshot: &SimulationSnapshot) -> String {
        format!("Gen: {:>6} | Cells: {:>6} | Status: {}",
                snapshot.generation,
                snapshot.population,
                if snapshot.is_running { "RUNNING" } else { "PAUSED" })
    }
    
    /// Repeat character - EntTS style
    fn repeat_char(&self, c: char, count: i32) -> String {
        std::iter::repeat(c).take(count as usize).collect()
    }
    
    /// Get render config
    pub fn get_render_config(&self) -> &RenderConfig {
        &self.config
    }
    
    /// Set render config
    pub fn set_render_config(&mut self, config: RenderConfig) {
        self.config = config;
    }
}