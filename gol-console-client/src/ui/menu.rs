use anyhow::Result;
use ratatui::{
    backend::Backend,
    layout::{Alignment, Constraint, Direction, Layout, Rect},
    style::{Color, Style},
    text::{Line, Span},
    widgets::{Block, Borders, Clear, List, ListItem, Paragraph, Wrap},
    Frame,
};

#[derive(Debug, Clone)]
pub enum MenuType {
    Main,
    Patterns,
    Backends,
    Settings,
    About,
}

pub struct MenuSystem {
    current_menu: Option<MenuType>,
    selected_index: usize,
    available_patterns: Vec<String>,
    available_backends: Vec<String>,
}

impl MenuSystem {
    pub fn new() -> Self {
        Self {
            current_menu: None,
            selected_index: 0,
            available_patterns: vec![
                "blinker".to_string(),
                "glider".to_string(),
                "beacon".to_string(),
                "toad".to_string(),
                "block".to_string(),
                "glider-gun".to_string(),
            ],
            available_backends: vec![
                "bevy".to_string(),
                "entt".to_string(),
                "flecs".to_string(),
            ],
        }
    }
    
    pub fn show_menu(&mut self, menu_type: MenuType) {
        self.current_menu = Some(menu_type);
        self.selected_index = 0;
    }
    
    pub fn hide_menu(&mut self) {
        self.current_menu = None;
        self.selected_index = 0;
    }
    
    pub fn is_menu_active(&self) -> bool {
        self.current_menu.is_some()
    }
    
    pub fn get_current_menu(&self) -> Option<MenuType> {
        self.current_menu.clone()
    }
    
    pub fn move_selection(&mut self, direction: i32) {
        if let Some(menu_type) = &self.current_menu {
            let max_items = match menu_type {
                MenuType::Main => 5,
                MenuType::Patterns => self.available_patterns.len(),
                MenuType::Backends => self.available_backends.len(),
                MenuType::Settings => 4,
                MenuType::About => 1,
            };
            
            if direction > 0 && self.selected_index < max_items - 1 {
                self.selected_index += 1;
            } else if direction < 0 && self.selected_index > 0 {
                self.selected_index -= 1;
            }
        }
    }
    
    pub fn get_selected_item(&self) -> Option<String> {
        if let Some(menu_type) = &self.current_menu {
            match menu_type {
                MenuType::Main => {
                    let items = vec!["New Simulation", "Load Pattern", "Switch Backend", "Settings", "About"];
                    items.get(self.selected_index).map(|s| s.to_string())
                }
                MenuType::Patterns => {
                    self.available_patterns.get(self.selected_index).cloned()
                }
                MenuType::Backends => {
                    self.available_backends.get(self.selected_index).cloned()
                }
                MenuType::Settings => {
                    let items = vec!["Auto-step Speed", "Grid Colors", "Viewport", "Keybindings"];
                    items.get(self.selected_index).map(|s| s.to_string())
                }
                MenuType::About => Some("Close".to_string()),
            }
        } else {
            None
        }
    }
    
    pub fn render(&self, frame: &mut Frame, area: Rect) {
        if let Some(menu_type) = &self.current_menu {
            // Create a centered popup area
            let popup_area = self.centered_rect(60, 70, area);
            
            frame.render_widget(Clear, popup_area);
            
            match menu_type {
                MenuType::Main => self.render_main_menu(frame, popup_area),
                MenuType::Patterns => self.render_patterns_menu(frame, popup_area),
                MenuType::Backends => self.render_backends_menu(frame, popup_area),
                MenuType::Settings => self.render_settings_menu(frame, popup_area),
                MenuType::About => self.render_about_menu(frame, popup_area),
            }
        }
    }
    
    fn render_main_menu(&self, frame: &mut Frame, area: Rect) {
        let items = vec![
            "New Simulation",
            "Load Pattern",
            "Switch Backend",
            "Settings",
            "About",
        ];
        
        let list_items: Vec<ListItem> = items
            .iter()
            .enumerate()
            .map(|(i, &item)| {
                let style = if i == self.selected_index {
                    Style::default().fg(Color::Yellow).bg(Color::DarkGray)
                } else {
                    Style::default().fg(Color::White)
                };
                ListItem::new(item).style(style)
            })
            .collect();
        
        let list = List::new(list_items)
            .block(
                Block::default()
                    .title("Main Menu")
                    .borders(Borders::ALL)
                    .style(Style::default().fg(Color::White))
            )
            .highlight_style(Style::default().fg(Color::Yellow))
            .highlight_symbol(">> ");
        
        frame.render_widget(list, area);
    }
    
    fn render_patterns_menu(&self, frame: &mut Frame, area: Rect) {
        let list_items: Vec<ListItem> = self.available_patterns
            .iter()
            .enumerate()
            .map(|(i, pattern)| {
                let style = if i == self.selected_index {
                    Style::default().fg(Color::Yellow).bg(Color::DarkGray)
                } else {
                    Style::default().fg(Color::White)
                };
                ListItem::new(pattern.as_str()).style(style)
            })
            .collect();
        
        let list = List::new(list_items)
            .block(
                Block::default()
                    .title("Select Pattern")
                    .borders(Borders::ALL)
                    .style(Style::default().fg(Color::White))
            )
            .highlight_style(Style::default().fg(Color::Yellow))
            .highlight_symbol(">> ");
        
        frame.render_widget(list, area);
    }
    
    fn render_backends_menu(&self, frame: &mut Frame, area: Rect) {
        let list_items: Vec<ListItem> = self.available_backends
            .iter()
            .enumerate()
            .map(|(i, backend)| {
                let style = if i == self.selected_index {
                    Style::default().fg(Color::Yellow).bg(Color::DarkGray)
                } else {
                    Style::default().fg(Color::White)
                };
                let description = match backend.as_str() {
                    "bevy" => "Bevy ECS Implementation",
                    "entt" => "EnTT ECS Implementation",
                    "flecs" => "Flecs ECS Implementation",
                    _ => backend,
                };
                ListItem::new(description).style(style)
            })
            .collect();
        
        let list = List::new(list_items)
            .block(
                Block::default()
                    .title("Select Backend")
                    .borders(Borders::ALL)
                    .style(Style::default().fg(Color::White))
            )
            .highlight_style(Style::default().fg(Color::Yellow))
            .highlight_symbol(">> ");
        
        frame.render_widget(list, area);
    }
    
    fn render_settings_menu(&self, frame: &mut Frame, area: Rect) {
        let items = vec![
            "Auto-step Speed",
            "Grid Colors",
            "Viewport Settings",
            "Keybinding Configuration",
        ];
        
        let list_items: Vec<ListItem> = items
            .iter()
            .enumerate()
            .map(|(i, &item)| {
                let style = if i == self.selected_index {
                    Style::default().fg(Color::Yellow).bg(Color::DarkGray)
                } else {
                    Style::default().fg(Color::White)
                };
                ListItem::new(item).style(style)
            })
            .collect();
        
        let list = List::new(list_items)
            .block(
                Block::default()
                    .title("Settings")
                    .borders(Borders::ALL)
                    .style(Style::default().fg(Color::White))
            )
            .highlight_style(Style::default().fg(Color::Yellow))
            .highlight_symbol(">> ");
        
        frame.render_widget(list, area);
    }
    
    fn render_about_menu(&self, frame: &mut Frame, area: Rect) {
        let about_text = vec![
            Line::from("Game of Life Console Client"),
            Line::from(""),
            Line::from("Version: 0.1.0"),
            Line::from(""),
            Line::from("A terminal-based client for Conway's Game of Life"),
            Line::from("supporting multiple ECS backend implementations:"),
            Line::from("• Bevy ECS (Rust)"),
            Line::from("• EnTT ECS (C++)"),
            Line::from("• Flecs ECS (C++)"),
            Line::from(""),
            Line::from("Features:"),
            Line::from("• Real-time grid visualization"),
            Line::from("• Pattern loading and management"),
            Line::from("• Interactive command interface"),
            Line::from("• Cross-platform gRPC communication"),
            Line::from(""),
            Line::from("Built with Rust, Tokio, and Ratatui"),
            Line::from(""),
            Line::from("Press Esc to close"),
        ];
        
        let paragraph = Paragraph::new(about_text)
            .block(
                Block::default()
                    .title("About")
                    .borders(Borders::ALL)
                    .style(Style::default().fg(Color::White))
            )
            .style(Style::default().fg(Color::White))
            .alignment(Alignment::Left)
            .wrap(Wrap { trim: true });
        
        frame.render_widget(paragraph, area);
    }
    
    fn centered_rect(&self, percent_x: u16, percent_y: u16, r: Rect) -> Rect {
        let popup_layout = Layout::default()
            .direction(Direction::Vertical)
            .constraints([
                Constraint::Percentage((100 - percent_y) / 2),
                Constraint::Percentage(percent_y),
                Constraint::Percentage((100 - percent_y) / 2),
            ])
            .split(r);
        
        Layout::default()
            .direction(Direction::Horizontal)
            .constraints([
                Constraint::Percentage((100 - percent_x) / 2),
                Constraint::Percentage(percent_x),
                Constraint::Percentage((100 - percent_x) / 2),
            ])
            .split(popup_layout[1])[1]
    }
    
    pub fn render_command_prompt(&self, frame: &mut Frame, area: Rect, prompt: &str) {
        if !prompt.is_empty() {
            let prompt_area = Rect {
                x: area.x,
                y: area.y + area.height - 1,
                width: area.width,
                height: 1,
            };
            
            let prompt_paragraph = Paragraph::new(prompt)
                .style(Style::default().fg(Color::Cyan))
                .block(Block::default().borders(Borders::NONE));
            
            frame.render_widget(prompt_paragraph, prompt_area);
        }
    }
    
    pub fn update_available_patterns(&mut self, patterns: Vec<String>) {
        self.available_patterns = patterns;
    }
    
    pub fn get_menu_help(&self) -> String {
        if let Some(menu_type) = &self.current_menu {
            match menu_type {
                MenuType::Main => "Navigate: ↑/↓, Select: Enter, Back: Esc",
                MenuType::Patterns => "Navigate: ↑/↓, Load: Enter, Back: Esc",
                MenuType::Backends => "Navigate: ↑/↓, Switch: Enter, Back: Esc",
                MenuType::Settings => "Navigate: ↑/↓, Configure: Enter, Back: Esc",
                MenuType::About => "Press Esc to close",
            }.to_string()
        } else {
            "Press m for menu, h for help, q to quit".to_string()
        }
    }
}