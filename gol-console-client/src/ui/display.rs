use anyhow::Result;
use ratatui::{
    backend::Backend,
    layout::{Constraint, Direction, Layout, Rect},
    style::{Color, Style},
    text::{Line, Span},
    widgets::{Block, Borders, Paragraph, Clear},
    Frame,
};
use crate::client::game_of_life::{Cell, SimulationResponse};
use std::collections::HashMap;

pub struct GridDisplay {
    width: u16,
    height: u16,
    live_cells: HashMap<(i32, i32), bool>,
    generation: i64,
    live_count: i64,
    viewport_x: i32,
    viewport_y: i32,
    zoom: f32,
}

impl GridDisplay {
    pub fn new() -> Self {
        Self {
            width: 80,
            height: 24,
            live_cells: HashMap::new(),
            generation: 0,
            live_count: 0,
            viewport_x: 0,
            viewport_y: 0,
            zoom: 1.0,
        }
    }
    
    pub fn update_from_simulation(&mut self, simulation: &SimulationResponse) {
        self.live_cells.clear();
        self.generation = simulation.generation;
        self.live_count = simulation.live_cells;
        
        for cell in &simulation.cells {
            if cell.alive {
                self.live_cells.insert((cell.x, cell.y), true);
            }
        }
    }
    
    pub fn update_terminal_size(&mut self, width: u16, height: u16) {
        self.width = width;
        self.height = height;
    }
    
    pub fn set_viewport(&mut self, x: i32, y: i32) {
        self.viewport_x = x;
        self.viewport_y = y;
    }
    
    pub fn move_viewport(&mut self, dx: i32, dy: i32) {
        self.viewport_x += dx;
        self.viewport_y += dy;
    }
    
    pub fn set_zoom(&mut self, zoom: f32) {
        self.zoom = zoom.max(0.5).min(4.0);
    }
    
    pub fn render(&self, frame: &mut Frame, area: Rect) {
        let chunks = Layout::default()
            .direction(Direction::Vertical)
            .constraints([
                Constraint::Length(1), // Status bar
                Constraint::Min(0),    // Grid area
                Constraint::Length(1), // Command hint
            ])
            .split(area);
        
        self.render_status_bar(frame, chunks[0]);
        self.render_grid(frame, chunks[1]);
        self.render_command_hint(frame, chunks[2]);
    }
    
    fn render_status_bar(&self, frame: &mut Frame, area: Rect) {
        let status_text = format!(
            "Generation: {} | Live Cells: {} | Viewport: ({}, {}) | Zoom: {:.1}x",
            self.generation, self.live_count, self.viewport_x, self.viewport_y, self.zoom
        );
        
        let status = Paragraph::new(status_text)
            .style(Style::default().fg(Color::Yellow))
            .block(Block::default().borders(Borders::NONE));
        
        frame.render_widget(status, area);
    }
    
    fn render_grid(&self, frame: &mut Frame, area: Rect) {
        let block = Block::default()
            .title("Game of Life")
            .borders(Borders::ALL)
            .style(Style::default().fg(Color::White));
        
        frame.render_widget(block, area);
        
        let inner_area = Rect {
            x: area.x + 1,
            y: area.y + 1,
            width: area.width.saturating_sub(2),
            height: area.height.saturating_sub(2),
        };
        
        let grid_lines = self.generate_grid_lines(inner_area);
        let grid_paragraph = Paragraph::new(grid_lines)
            .style(Style::default().fg(Color::White));
        
        frame.render_widget(grid_paragraph, inner_area);
    }
    
    fn generate_grid_lines(&self, area: Rect) -> Vec<Line> {
        let mut lines = Vec::new();
        let cell_size = (1.0 / self.zoom) as i32;
        
        for row in 0..area.height {
            let mut line_spans = Vec::new();
            let world_y = self.viewport_y + (row as i32 * cell_size);
            
            for col in 0..area.width {
                let world_x = self.viewport_x + (col as i32 * cell_size);
                
                let cell_char = if self.live_cells.contains_key(&(world_x, world_y)) {
                    '●'
                } else {
                    '·'
                };
                
                let cell_style = if self.live_cells.contains_key(&(world_x, world_y)) {
                    Style::default().fg(Color::Green)
                } else {
                    Style::default().fg(Color::DarkGray)
                };
                
                line_spans.push(Span::styled(cell_char.to_string(), cell_style));
            }
            
            lines.push(Line::from(line_spans));
        }
        
        lines
    }
    
    fn render_command_hint(&self, frame: &mut Frame, area: Rect) {
        let hint_text = "Commands: q=quit, h=help, r=run, s=step, p=pause, arrows=move, +/-=zoom";
        
        let hint = Paragraph::new(hint_text)
            .style(Style::default().fg(Color::Cyan))
            .block(Block::default().borders(Borders::NONE));
        
        frame.render_widget(hint, area);
    }
    
    pub fn render_help(&self, frame: &mut Frame, area: Rect) {
        let help_text = vec![
            Line::from("Game of Life Console Client - Help"),
            Line::from(""),
            Line::from("Navigation:"),
            Line::from("  Arrow Keys    - Move viewport"),
            Line::from("  +/-           - Zoom in/out"),
            Line::from("  Home          - Reset viewport to origin"),
            Line::from(""),
            Line::from("Simulation:"),
            Line::from("  r             - Run simulation"),
            Line::from("  s             - Step one generation"),
            Line::from("  p             - Pause simulation"),
            Line::from("  c             - Clear grid"),
            Line::from("  l             - Load pattern"),
            Line::from(""),
            Line::from("Interface:"),
            Line::from("  h             - Show/hide this help"),
            Line::from("  q             - Quit application"),
            Line::from("  Enter         - Command mode"),
            Line::from(""),
            Line::from("Press any key to close help"),
        ];
        
        let help_block = Block::default()
            .title("Help")
            .borders(Borders::ALL)
            .style(Style::default().fg(Color::Yellow));
        
        let help_paragraph = Paragraph::new(help_text)
            .block(help_block)
            .style(Style::default().fg(Color::White));
        
        frame.render_widget(Clear, area);
        frame.render_widget(help_paragraph, area);
    }
    
    pub fn center_on_live_cells(&mut self) {
        if self.live_cells.is_empty() {
            return;
        }
        
        let mut min_x = i32::MAX;
        let mut max_x = i32::MIN;
        let mut min_y = i32::MAX;
        let mut max_y = i32::MIN;
        
        for &(x, y) in self.live_cells.keys() {
            min_x = min_x.min(x);
            max_x = max_x.max(x);
            min_y = min_y.min(y);
            max_y = max_y.max(y);
        }
        
        let center_x = (min_x + max_x) / 2;
        let center_y = (min_y + max_y) / 2;
        
        self.viewport_x = center_x - (self.width as i32 / 2);
        self.viewport_y = center_y - (self.height as i32 / 2);
    }
    
    pub fn get_cell_at_screen_pos(&self, screen_x: u16, screen_y: u16) -> (i32, i32) {
        let cell_size = (1.0 / self.zoom) as i32;
        let world_x = self.viewport_x + (screen_x as i32 * cell_size);
        let world_y = self.viewport_y + (screen_y as i32 * cell_size);
        (world_x, world_y)
    }
    
    pub fn get_viewport_info(&self) -> (i32, i32, f32) {
        (self.viewport_x, self.viewport_y, self.zoom)
    }
    
    pub fn get_stats(&self) -> (i64, i64) {
        (self.generation, self.live_count)
    }
}