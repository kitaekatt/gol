use anyhow::Result;
use ratatui::{
    backend::CrosstermBackend,
    Terminal,
};
use crossterm::{
    event::{self, Event, KeyCode},
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
    ExecutableCommand,
};
use std::io::{self, stdout};
use std::time::{Duration, Instant};
use tokio::time;

pub mod interactive;
pub mod display;
pub mod input;
pub mod menu;

use display::GridDisplay;
use input::{InputHandler, InputAction};
use menu::{MenuSystem, MenuType};
use crate::client::GameOfLifeClient;

pub struct TerminalUI {
    terminal: Terminal<CrosstermBackend<io::Stdout>>,
    display: GridDisplay,
    input_handler: InputHandler,
    menu_system: MenuSystem,
    client: GameOfLifeClient,
    last_update: Instant,
    auto_step_interval: Duration,
    running: bool,
}

impl TerminalUI {
    pub fn new() -> Result<Self> {
        enable_raw_mode()?;
        stdout().execute(EnterAlternateScreen)?;
        let backend = CrosstermBackend::new(stdout());
        let terminal = Terminal::new(backend)?;
        
        let client = GameOfLifeClient::for_backend("bevy");
        
        Ok(Self {
            terminal,
            display: GridDisplay::new(),
            input_handler: InputHandler::new(),
            menu_system: MenuSystem::new(),
            client,
            last_update: Instant::now(),
            auto_step_interval: Duration::from_millis(1000),
            running: false,
        })
    }
    
    pub async fn run_interactive(&mut self) -> Result<()> {
        loop {
            let size = self.terminal.size()?;
            let (term_width, term_height) = (size.width, size.height);
            self.display.update_terminal_size(term_width, term_height);
            
            self.terminal.draw(|f| {
                let size = f.area();
                
                if self.menu_system.is_menu_active() {
                    self.display.render(f, size);
                    self.menu_system.render(f, size);
                } else if self.input_handler.is_help_shown() {
                    self.display.render_help(f, size);
                } else {
                    self.display.render(f, size);
                }
                
                if self.input_handler.is_command_mode() {
                    let prompt = self.input_handler.get_command_prompt();
                    self.menu_system.render_command_prompt(f, size, &prompt);
                }
            })?;
            
            if event::poll(Duration::from_millis(100))? {
                if let Event::Key(key) = event::read()? {
                    if let Some(action) = self.input_handler.handle_key_event(key)? {
                        if self.handle_action(action).await? {
                            break;
                        }
                    }
                }
            }
            
            if self.running && self.last_update.elapsed() >= self.auto_step_interval {
                self.step_simulation().await?;
                self.last_update = Instant::now();
            }
        }
        
        Ok(())
    }
    
    async fn handle_action(&mut self, action: InputAction) -> Result<bool> {
        match action {
            InputAction::Quit => return Ok(true),
            
            InputAction::ShowHelp(show) => {
                // Help display is handled by the input handler
            }
            
            InputAction::MoveViewport(dx, dy) => {
                self.display.move_viewport(dx, dy);
            }
            
            InputAction::Zoom(factor) => {
                let (_, _, current_zoom) = self.display.get_viewport_info();
                self.display.set_zoom(current_zoom * factor);
            }
            
            InputAction::ResetViewport => {
                self.display.set_viewport(0, 0);
                self.display.set_zoom(1.0);
            }
            
            InputAction::CenterOnCells => {
                self.display.center_on_live_cells();
            }
            
            InputAction::StepSimulation => {
                self.step_simulation().await?;
            }
            
            InputAction::RunSimulation => {
                self.running = !self.running;
                if self.running {
                    self.last_update = Instant::now();
                }
            }
            
            InputAction::PauseSimulation => {
                self.running = false;
            }
            
            InputAction::LoadPattern(pattern) => {
                self.load_pattern(&pattern).await?;
            }
            
            InputAction::SwitchBackend(backend) => {
                self.client = GameOfLifeClient::for_backend(&backend);
            }
            
            InputAction::CommandMode => {
                // Command mode is handled by the input handler
            }
            
            InputAction::ExecuteCommand(command) => {
                let result = self.input_handler.execute_command(&command, &mut self.client).await?;
                println!("{}", result); // In a real UI, this would show in a status area
            }
            
            InputAction::ClearGrid => {
                self.display = GridDisplay::new();
            }
            
            _ => {}
        }
        
        Ok(false)
    }
    
    async fn step_simulation(&mut self) -> Result<()> {
        let mut client = self.client.clone();
        
        match client.connect().await {
            Ok(_) => {
                match client.step_simulation("default".to_string(), 1).await {
                    Ok(_) => {
                        match client.get_simulation("default".to_string()).await {
                            Ok(sim) => {
                                self.display.update_from_simulation(&sim);
                            }
                            Err(_) => {
                                // Create simulation if it doesn't exist
                                let _ = client.create_simulation(100, 50, None).await;
                            }
                        }
                    }
                    Err(_) => {
                        // Create simulation if step fails
                        let _ = client.create_simulation(100, 50, None).await;
                    }
                }
            }
            Err(_) => {
                // Server not available
            }
        }
        
        Ok(())
    }
    
    async fn load_pattern(&mut self, pattern_name: &str) -> Result<()> {
        // This would load a pattern from the patterns directory
        // For now, we'll just create a simple pattern
        println!("Loading pattern: {}", pattern_name);
        Ok(())
    }
    
    pub fn with_client(mut self, client: GameOfLifeClient) -> Self {
        self.client = client;
        self
    }
    
    pub fn set_auto_step_interval(&mut self, interval: Duration) {
        self.auto_step_interval = interval;
    }
}

impl Drop for TerminalUI {
    fn drop(&mut self) {
        let _ = disable_raw_mode();
        let _ = stdout().execute(LeaveAlternateScreen);
    }
}