use bevy::prelude::*;
use uuid::Uuid;
use std::collections::HashMap;
use std::time::SystemTime;
use crate::components::{Position, CellState};

#[derive(Resource)]
pub struct Simulations {
    pub simulations: HashMap<String, SimulationData>,
    pub server_start_time: SystemTime,
}

impl Default for Simulations {
    fn default() -> Self {
        Self::new()
    }
}

#[derive(Debug, Clone)]
pub struct SimulationData {
    pub id: String,
    pub generation: u64,
    pub width: i32,
    pub height: i32,
    pub cells: HashMap<(i32, i32), CellState>,
    pub is_running: bool,
    pub created_at: SystemTime,
}

impl Simulations {
    pub fn new() -> Self {
        Self {
            simulations: HashMap::new(),
            server_start_time: SystemTime::now(),
        }
    }
    
    pub fn create_simulation(&mut self, width: i32, height: i32, initial_pattern: Option<String>) -> String {
        let id = Uuid::new_v4().to_string();
        let simulation = SimulationData {
            id: id.clone(),
            generation: 0,
            width,
            height,
            cells: HashMap::new(),
            is_running: false,
            created_at: SystemTime::now(),
        };
        
        self.simulations.insert(id.clone(), simulation);
        id
    }
    
    pub fn get_simulation(&self, id: &str) -> Option<&SimulationData> {
        self.simulations.get(id)
    }
    
    pub fn get_simulation_mut(&mut self, id: &str) -> Option<&mut SimulationData> {
        self.simulations.get_mut(id)
    }
    
    pub fn delete_simulation(&mut self, id: &str) -> bool {
        self.simulations.remove(id).is_some()
    }
    
    pub fn uptime_seconds(&self) -> i64 {
        SystemTime::now()
            .duration_since(self.server_start_time)
            .unwrap_or_default()
            .as_secs() as i64
    }
}

impl SimulationData {
    pub fn set_cells(&mut self, cells: &[(i32, i32)]) {
        self.cells.clear();
        for (x, y) in cells {
            if *x >= 0 && *x < self.width && *y >= 0 && *y < self.height {
                self.cells.insert((*x, *y), CellState::new());
            }
        }
    }
    
    pub fn get_live_cells(&self) -> Vec<(i32, i32)> {
        self.cells
            .iter()
            .filter(|(_, cell)| cell.alive)
            .map(|((x, y), _)| (*x, *y))
            .collect()
    }
    
    pub fn get_live_cell_count(&self) -> i64 {
        self.cells.values().filter(|cell| cell.alive).count() as i64
    }
    
    pub fn add_pattern(&mut self, pattern: &[(i32, i32)], offset_x: i32, offset_y: i32) -> i32 {
        let mut cells_added = 0;
        
        for (x, y) in pattern {
            let new_x = x + offset_x;
            let new_y = y + offset_y;
            
            if new_x >= 0 && new_x < self.width && new_y >= 0 && new_y < self.height {
                if !self.cells.contains_key(&(new_x, new_y)) {
                    self.cells.insert((new_x, new_y), CellState::new());
                    cells_added += 1;
                }
            }
        }
        
        cells_added
    }
}