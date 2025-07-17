use anyhow::Result;
use crossterm::event::{Event, KeyCode, KeyEvent, KeyModifiers};
use std::collections::VecDeque;
use crate::client::GameOfLifeClient;
use crate::commands::{simulation, pattern, control};

#[derive(Debug, Clone)]
pub enum InputAction {
    Quit,
    Help,
    ShowHelp(bool),
    MoveViewport(i32, i32),
    Zoom(f32),
    ResetViewport,
    StepSimulation,
    RunSimulation,
    PauseSimulation,
    ClearGrid,
    LoadPattern(String),
    CommandMode,
    ExecuteCommand(String),
    CenterOnCells,
    SavePattern(String),
    SwitchBackend(String),
}

pub struct InputHandler {
    command_mode: bool,
    command_buffer: String,
    command_history: VecDeque<String>,
    history_index: usize,
    show_help: bool,
}

impl InputHandler {
    pub fn new() -> Self {
        Self {
            command_mode: false,
            command_buffer: String::new(),
            command_history: VecDeque::new(),
            history_index: 0,
            show_help: false,
        }
    }
    
    pub fn handle_key_event(&mut self, key: KeyEvent) -> Result<Option<InputAction>> {
        if self.command_mode {
            self.handle_command_mode_key(key)
        } else {
            self.handle_normal_mode_key(key)
        }
    }
    
    fn handle_normal_mode_key(&mut self, key: KeyEvent) -> Result<Option<InputAction>> {
        match key.code {
            KeyCode::Char('q') | KeyCode::Char('Q') => Ok(Some(InputAction::Quit)),
            KeyCode::Char('h') | KeyCode::Char('H') => {
                self.show_help = !self.show_help;
                Ok(Some(InputAction::ShowHelp(self.show_help)))
            }
            KeyCode::Char('r') | KeyCode::Char('R') => Ok(Some(InputAction::RunSimulation)),
            KeyCode::Char('s') | KeyCode::Char('S') => Ok(Some(InputAction::StepSimulation)),
            KeyCode::Char('p') | KeyCode::Char('P') => Ok(Some(InputAction::PauseSimulation)),
            KeyCode::Char('c') | KeyCode::Char('C') => Ok(Some(InputAction::ClearGrid)),
            KeyCode::Char('l') | KeyCode::Char('L') => Ok(Some(InputAction::LoadPattern("glider".to_string()))),
            KeyCode::Char('o') | KeyCode::Char('O') => Ok(Some(InputAction::CenterOnCells)),
            KeyCode::Enter => {
                self.command_mode = true;
                self.command_buffer.clear();
                Ok(Some(InputAction::CommandMode))
            }
            KeyCode::Home => Ok(Some(InputAction::ResetViewport)),
            KeyCode::Up => Ok(Some(InputAction::MoveViewport(0, -1))),
            KeyCode::Down => Ok(Some(InputAction::MoveViewport(0, 1))),
            KeyCode::Left => Ok(Some(InputAction::MoveViewport(-1, 0))),
            KeyCode::Right => Ok(Some(InputAction::MoveViewport(1, 0))),
            KeyCode::Char('+') | KeyCode::Char('=') => Ok(Some(InputAction::Zoom(1.2))),
            KeyCode::Char('-') | KeyCode::Char('_') => Ok(Some(InputAction::Zoom(0.8))),
            KeyCode::Char('1') => Ok(Some(InputAction::SwitchBackend("bevy".to_string()))),
            KeyCode::Char('2') => Ok(Some(InputAction::SwitchBackend("entt".to_string()))),
            KeyCode::Char('3') => Ok(Some(InputAction::SwitchBackend("flecs".to_string()))),
            KeyCode::Esc => {
                if self.show_help {
                    self.show_help = false;
                    Ok(Some(InputAction::ShowHelp(false)))
                } else {
                    Ok(None)
                }
            }
            _ => Ok(None),
        }
    }
    
    fn handle_command_mode_key(&mut self, key: KeyEvent) -> Result<Option<InputAction>> {
        match key.code {
            KeyCode::Enter => {
                let command = self.command_buffer.trim().to_string();
                if !command.is_empty() {
                    self.add_to_history(command.clone());
                    self.command_mode = false;
                    self.command_buffer.clear();
                    Ok(Some(InputAction::ExecuteCommand(command)))
                } else {
                    self.command_mode = false;
                    Ok(None)
                }
            }
            KeyCode::Esc => {
                self.command_mode = false;
                self.command_buffer.clear();
                Ok(None)
            }
            KeyCode::Backspace => {
                self.command_buffer.pop();
                Ok(None)
            }
            KeyCode::Up => {
                if !self.command_history.is_empty() && self.history_index > 0 {
                    self.history_index -= 1;
                    self.command_buffer = self.command_history[self.history_index].clone();
                }
                Ok(None)
            }
            KeyCode::Down => {
                if !self.command_history.is_empty() && self.history_index < self.command_history.len() - 1 {
                    self.history_index += 1;
                    self.command_buffer = self.command_history[self.history_index].clone();
                } else {
                    self.command_buffer.clear();
                }
                Ok(None)
            }
            KeyCode::Char(c) => {
                self.command_buffer.push(c);
                Ok(None)
            }
            _ => Ok(None),
        }
    }
    
    fn add_to_history(&mut self, command: String) {
        if self.command_history.len() >= 50 {
            self.command_history.pop_front();
        }
        self.command_history.push_back(command);
        self.history_index = self.command_history.len();
    }
    
    pub fn is_command_mode(&self) -> bool {
        self.command_mode
    }
    
    pub fn get_command_buffer(&self) -> &str {
        &self.command_buffer
    }
    
    pub fn is_help_shown(&self) -> bool {
        self.show_help
    }
    
    pub async fn execute_command(&mut self, command: &str, client: &mut GameOfLifeClient) -> Result<String> {
        let parts: Vec<&str> = command.split_whitespace().collect();
        if parts.is_empty() {
            return Ok("No command entered".to_string());
        }
        
        let cmd = parts[0].to_lowercase();
        let args = &parts[1..];
        
        match cmd.as_str() {
            "help" | "h" => Ok(self.get_help_text()),
            "quit" | "q" | "exit" => Ok("Quitting...".to_string()),
            
            "create" | "new" => {
                if args.len() >= 2 {
                    let width = args[0].parse::<i32>().unwrap_or(50);
                    let height = args[1].parse::<i32>().unwrap_or(30);
                    let pattern = args.get(2).map(|s| s.to_string());
                    
                    let mut sim_cmd = simulation::SimulationCommands::new(client.clone());
                    match sim_cmd.create(width, height, pattern).await {
                        Ok(response) => Ok(format!("Created simulation: {}", response.id)),
                        Err(e) => Ok(format!("Error creating simulation: {}", e)),
                    }
                } else {
                    Ok("Usage: create <width> <height> [pattern]".to_string())
                }
            }
            
            "step" | "s" => {
                let steps = args.get(0).and_then(|s| s.parse::<i32>().ok()).unwrap_or(1);
                let sim_id = args.get(1).unwrap_or(&"default").to_string();
                
                let mut sim_cmd = simulation::SimulationCommands::new(client.clone());
                match sim_cmd.step(sim_id, steps).await {
                    Ok(response) => Ok(format!("Stepped to generation {}", response.generation)),
                    Err(e) => Ok(format!("Error stepping simulation: {}", e)),
                }
            }
            
            "load" | "l" => {
                if !args.is_empty() {
                    let pattern_name = args[0];
                    let x = args.get(1).and_then(|s| s.parse::<i32>().ok()).unwrap_or(0);
                    let y = args.get(2).and_then(|s| s.parse::<i32>().ok()).unwrap_or(0);
                    
                    let mut pattern_cmd = pattern::PatternCommands::new(client.clone());
                    let pattern_file = format!("../patterns/{}.json", pattern_name);
                    
                    match pattern_cmd.load_from_file("default".to_string(), &pattern_file, x, y).await {
                        Ok(_) => Ok(format!("Loaded pattern: {}", pattern_name)),
                        Err(e) => Ok(format!("Error loading pattern: {}", e)),
                    }
                } else {
                    Ok("Usage: load <pattern_name> [x] [y]".to_string())
                }
            }
            
            "run" | "r" => {
                let mut control_cmd = control::ControlCommands::new(client.clone());
                match control_cmd.play(Some("default".to_string())).await {
                    Ok(_) => Ok("Started simulation".to_string()),
                    Err(e) => Ok(format!("Error running simulation: {}", e)),
                }
            }
            
            "status" | "stat" => {
                let mut sim_cmd = simulation::SimulationCommands::new(client.clone());
                match sim_cmd.status().await {
                    Ok(status) => Ok(status),
                    Err(e) => Ok(format!("Error getting status: {}", e)),
                }
            }
            
            "backend" | "be" => {
                if !args.is_empty() {
                    let backend = args[0];
                    let mut control_cmd = control::ControlCommands::new(client.clone());
                    match control_cmd.switch_backend(backend) {
                        Ok(_) => Ok(format!("Switched to {} backend", backend)),
                        Err(e) => Ok(format!("Error switching backend: {}", e)),
                    }
                } else {
                    Ok("Usage: backend <bevy|entt|flecs>".to_string())
                }
            }
            
            "clear" | "c" => {
                Ok("Grid cleared (not implemented)".to_string())
            }
            
            _ => Ok(format!("Unknown command: {}. Type 'help' for available commands.", cmd)),
        }
    }
    
    fn get_help_text(&self) -> String {
        let help = vec![
            "Available Commands:",
            "",
            "Simulation:",
            "  create <w> <h> [pattern] - Create new simulation",
            "  step [count] [sim_id]    - Step simulation",
            "  run [sim_id]             - Run simulation",
            "  status                   - Get server status",
            "  clear                    - Clear grid",
            "",
            "Patterns:",
            "  load <name> [x] [y]      - Load pattern at position",
            "",
            "Control:",
            "  backend <name>           - Switch backend (bevy|entt|flecs)",
            "  help                     - Show this help",
            "  quit                     - Exit application",
            "",
            "Keyboard Shortcuts:",
            "  q - quit, h - help, r - run, s - step, p - pause",
            "  arrows - move view, +/- - zoom, Enter - command mode",
            "  1/2/3 - switch backend, o - center on cells",
        ];
        
        help.join("\n")
    }
    
    pub fn get_command_prompt(&self) -> String {
        if self.command_mode {
            format!("> {}", self.command_buffer)
        } else {
            String::new()
        }
    }
}