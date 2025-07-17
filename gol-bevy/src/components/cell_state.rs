use bevy::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Component, Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct CellState {
    pub alive: bool,
    pub generation: u64,
    pub neighbor_count: u8,
}

impl CellState {
    pub fn new() -> Self {
        Self {
            alive: true,
            generation: 0,
            neighbor_count: 0,
        }
    }
    
    pub fn with_generation(generation: u64) -> Self {
        Self {
            alive: true,
            generation,
            neighbor_count: 0,
        }
    }
    
    pub fn with_neighbors(neighbor_count: u8) -> Self {
        Self {
            alive: true,
            generation: 0,
            neighbor_count,
        }
    }
    
    pub fn with_generation_and_neighbors(generation: u64, neighbor_count: u8) -> Self {
        Self {
            alive: true,
            generation,
            neighbor_count,
        }
    }
    
    pub fn should_survive(&self) -> bool {
        self.alive && (self.neighbor_count == 2 || self.neighbor_count == 3)
    }
    
    pub fn should_be_born(&self) -> bool {
        !self.alive && self.neighbor_count == 3
    }
    
    pub fn next_generation(&self) -> Self {
        Self {
            alive: self.should_survive() || self.should_be_born(),
            generation: self.generation + 1,
            neighbor_count: 0, // Reset for next calculation
        }
    }
}

impl Default for CellState {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_cell_state_creation() {
        let cell = CellState::new();
        assert_eq!(cell.alive, true);
        assert_eq!(cell.generation, 0);
        assert_eq!(cell.neighbor_count, 0);
    }
    
    #[test]
    fn test_cell_state_with_generation() {
        let cell = CellState::with_generation(5);
        assert_eq!(cell.alive, true);
        assert_eq!(cell.generation, 5);
        assert_eq!(cell.neighbor_count, 0);
    }
    
    #[test]
    fn test_cell_state_with_neighbors() {
        let cell = CellState::with_neighbors(3);
        assert_eq!(cell.alive, true);
        assert_eq!(cell.generation, 0);
        assert_eq!(cell.neighbor_count, 3);
    }
    
    #[test]
    fn test_cell_state_game_of_life_rules() {
        // Test survival with 2 neighbors
        let mut cell = CellState::with_neighbors(2);
        cell.alive = true;
        assert!(cell.should_survive());
        
        // Test survival with 3 neighbors
        cell.neighbor_count = 3;
        assert!(cell.should_survive());
        
        // Test death with 1 neighbor
        cell.neighbor_count = 1;
        assert!(!cell.should_survive());
        
        // Test death with 4 neighbors
        cell.neighbor_count = 4;
        assert!(!cell.should_survive());
        
        // Test birth with 3 neighbors
        cell.alive = false;
        cell.neighbor_count = 3;
        assert!(cell.should_be_born());
        
        // Test no birth with 2 neighbors
        cell.neighbor_count = 2;
        assert!(!cell.should_be_born());
    }
    
    #[test]
    fn test_next_generation() {
        // Test living cell with 2 neighbors survives
        let cell = CellState::with_generation_and_neighbors(1, 2);
        let next = cell.next_generation();
        assert!(next.alive);
        assert_eq!(next.generation, 2);
        assert_eq!(next.neighbor_count, 0);
        
        // Test living cell with 1 neighbor dies
        let cell = CellState::with_generation_and_neighbors(1, 1);
        let next = cell.next_generation();
        assert!(!next.alive);
        assert_eq!(next.generation, 2);
        
        // Test dead cell with 3 neighbors comes to life
        let mut cell = CellState::with_generation_and_neighbors(1, 3);
        cell.alive = false;
        let next = cell.next_generation();
        assert!(next.alive);
        assert_eq!(next.generation, 2);
    }
    
    #[test]
    fn test_cell_state_serialization() {
        let cell = CellState::with_generation_and_neighbors(5, 3);
        let json = serde_json::to_string(&cell).unwrap();
        let deserialized: CellState = serde_json::from_str(&json).unwrap();
        assert_eq!(cell, deserialized);
    }
    
    #[test]
    fn test_default_implementation() {
        let cell = CellState::default();
        let expected = CellState::new();
        assert_eq!(cell, expected);
    }
}