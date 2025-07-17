use bevy::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Component, Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct CellState {
    pub alive: bool,
    pub age: u32,
}

impl CellState {
    pub fn new(alive: bool) -> Self {
        Self { alive, age: 0 }
    }
    
    pub fn new_with_age(alive: bool, age: u32) -> Self {
        Self { alive, age }
    }
    
    pub fn increment_age(&mut self) {
        self.age = self.age.saturating_add(1);
    }
    
    pub fn reset_age(&mut self) {
        self.age = 0;
    }
    
    pub fn is_alive(&self) -> bool {
        self.alive
    }
    
    pub fn set_alive(&mut self, alive: bool) {
        self.alive = alive;
        if !alive {
            self.age = 0;
        }
    }
}

impl Default for CellState {
    fn default() -> Self {
        Self::new(false)
    }
}

#[derive(Component, Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct NeighborCount {
    pub count: u8,
    pub dirty: bool,
}

impl NeighborCount {
    pub fn new() -> Self {
        Self {
            count: 0,
            dirty: true,
        }
    }
    
    pub fn new_with_count(count: u8) -> Self {
        Self {
            count,
            dirty: false,
        }
    }
    
    pub fn set_count(&mut self, count: u8) {
        self.count = count;
        self.dirty = false;
    }
    
    pub fn mark_dirty(&mut self) {
        self.dirty = true;
    }
    
    pub fn is_dirty(&self) -> bool {
        self.dirty
    }
    
    pub fn get_count(&self) -> u8 {
        self.count
    }
}

impl Default for NeighborCount {
    fn default() -> Self {
        Self::new()
    }
}