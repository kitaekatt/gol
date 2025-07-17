use bevy::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Component, Clone, Copy, Debug, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Position {
    pub x: i32,
    pub y: i32,
}

impl Position {
    pub fn new(x: i32, y: i32) -> Self {
        Self { x, y }
    }
    
    pub fn neighbors(&self) -> [Position; 8] {
        [
            Position::new(self.x - 1, self.y - 1),
            Position::new(self.x - 1, self.y),
            Position::new(self.x - 1, self.y + 1),
            Position::new(self.x, self.y - 1),
            Position::new(self.x, self.y + 1),
            Position::new(self.x + 1, self.y - 1),
            Position::new(self.x + 1, self.y),
            Position::new(self.x + 1, self.y + 1),
        ]
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_position_creation() {
        let pos = Position::new(5, 10);
        assert_eq!(pos.x, 5);
        assert_eq!(pos.y, 10);
    }
    
    #[test]
    fn test_position_neighbors() {
        let pos = Position::new(0, 0);
        let neighbors = pos.neighbors();
        let expected = [
            Position::new(-1, -1),
            Position::new(-1, 0),
            Position::new(-1, 1),
            Position::new(0, -1),
            Position::new(0, 1),
            Position::new(1, -1),
            Position::new(1, 0),
            Position::new(1, 1),
        ];
        assert_eq!(neighbors, expected);
    }
    
    #[test]
    fn test_position_serialization() {
        let pos = Position::new(3, 7);
        let json = serde_json::to_string(&pos).unwrap();
        let deserialized: Position = serde_json::from_str(&json).unwrap();
        assert_eq!(pos, deserialized);
    }
    
    #[test]
    fn test_position_equality() {
        let pos1 = Position::new(1, 2);
        let pos2 = Position::new(1, 2);
        let pos3 = Position::new(2, 1);
        
        assert_eq!(pos1, pos2);
        assert_ne!(pos1, pos3);
    }
}