use bevy::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Component, Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct GridPosition {
    pub x: i32,
    pub y: i32,
}

impl GridPosition {
    pub fn new(x: i32, y: i32) -> Self {
        Self { x, y }
    }
    
    pub fn from_tuple(pos: (i32, i32)) -> Self {
        Self { x: pos.0, y: pos.1 }
    }
    
    pub fn to_tuple(self) -> (i32, i32) {
        (self.x, self.y)
    }
    
    pub fn get_neighbor_positions(&self) -> Vec<GridPosition> {
        vec![
            GridPosition::new(self.x - 1, self.y - 1), // top-left
            GridPosition::new(self.x,     self.y - 1), // top
            GridPosition::new(self.x + 1, self.y - 1), // top-right
            GridPosition::new(self.x - 1, self.y),     // left
            GridPosition::new(self.x + 1, self.y),     // right
            GridPosition::new(self.x - 1, self.y + 1), // bottom-left
            GridPosition::new(self.x,     self.y + 1), // bottom
            GridPosition::new(self.x + 1, self.y + 1), // bottom-right
        ]
    }
    
    pub fn get_neighbor_offsets() -> &'static [(i32, i32)] {
        &[
            (-1, -1), (0, -1), (1, -1),
            (-1,  0),          (1,  0),
            (-1,  1), (0,  1), (1,  1),
        ]
    }
    
    pub fn distance_to(&self, other: &GridPosition) -> f32 {
        let dx = (self.x - other.x) as f32;
        let dy = (self.y - other.y) as f32;
        (dx * dx + dy * dy).sqrt()
    }
    
    pub fn manhattan_distance_to(&self, other: &GridPosition) -> u32 {
        ((self.x - other.x).abs() + (self.y - other.y).abs()) as u32
    }
}

impl From<(i32, i32)> for GridPosition {
    fn from(pos: (i32, i32)) -> Self {
        Self::new(pos.0, pos.1)
    }
}

impl Into<(i32, i32)> for GridPosition {
    fn into(self) -> (i32, i32) {
        (self.x, self.y)
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct GridBoundary {
    pub width: i32,
    pub height: i32,
    pub wrap_edges: bool,
}

impl GridBoundary {
    pub fn new(width: i32, height: i32, wrap_edges: bool) -> Self {
        Self { width, height, wrap_edges }
    }
    
    pub fn infinite() -> Self {
        Self {
            width: i32::MAX,
            height: i32::MAX,
            wrap_edges: false,
        }
    }
    
    pub fn is_valid_position(&self, pos: &GridPosition) -> bool {
        if self.wrap_edges {
            true // All positions are valid with wrapping
        } else {
            pos.x >= 0 && pos.x < self.width && pos.y >= 0 && pos.y < self.height
        }
    }
    
    pub fn wrap_position(&self, pos: &GridPosition) -> GridPosition {
        if !self.wrap_edges {
            return *pos;
        }
        
        let wrapped_x = if pos.x < 0 {
            self.width + (pos.x % self.width)
        } else {
            pos.x % self.width
        };
        
        let wrapped_y = if pos.y < 0 {
            self.height + (pos.y % self.height)
        } else {
            pos.y % self.height
        };
        
        GridPosition::new(wrapped_x, wrapped_y)
    }
    
    pub fn clamp_position(&self, pos: &GridPosition) -> GridPosition {
        GridPosition::new(
            pos.x.max(0).min(self.width - 1),
            pos.y.max(0).min(self.height - 1),
        )
    }
}

impl Default for GridBoundary {
    fn default() -> Self {
        Self::new(100, 100, false)
    }
}

#[derive(Resource, Debug, Clone)]
pub struct SpatialGrid {
    pub cells: std::collections::HashMap<(i32, i32), Entity>,
    pub chunk_size: i32,
}

impl SpatialGrid {
    pub fn new() -> Self {
        Self {
            cells: std::collections::HashMap::new(),
            chunk_size: 32, // Chunk size for spatial partitioning
        }
    }
    
    pub fn new_with_chunk_size(chunk_size: i32) -> Self {
        Self {
            cells: std::collections::HashMap::new(),
            chunk_size,
        }
    }
    
    pub fn insert(&mut self, position: (i32, i32), entity: Entity) {
        self.cells.insert(position, entity);
    }
    
    pub fn remove(&mut self, position: &(i32, i32)) -> Option<Entity> {
        self.cells.remove(position)
    }
    
    pub fn get(&self, position: &(i32, i32)) -> Option<Entity> {
        self.cells.get(position).copied()
    }
    
    pub fn contains(&self, position: &(i32, i32)) -> bool {
        self.cells.contains_key(position)
    }
    
    pub fn get_neighbors(&self, position: (i32, i32)) -> Vec<Entity> {
        let mut neighbors = Vec::new();
        
        for &(dx, dy) in GridPosition::get_neighbor_offsets() {
            let neighbor_pos = (position.0 + dx, position.1 + dy);
            if let Some(entity) = self.get(&neighbor_pos) {
                neighbors.push(entity);
            }
        }
        
        neighbors
    }
    
    pub fn get_neighbors_in_chunk(&self, position: (i32, i32)) -> Vec<Entity> {
        let chunk_x = position.0 / self.chunk_size;
        let chunk_y = position.1 / self.chunk_size;
        
        let mut neighbors = Vec::new();
        
        // Check cells in the same chunk and adjacent chunks
        for chunk_dx in -1..=1 {
            for chunk_dy in -1..=1 {
                let check_chunk_x = chunk_x + chunk_dx;
                let check_chunk_y = chunk_y + chunk_dy;
                
                let chunk_start_x = check_chunk_x * self.chunk_size;
                let chunk_start_y = check_chunk_y * self.chunk_size;
                
                for x in chunk_start_x..(chunk_start_x + self.chunk_size) {
                    for y in chunk_start_y..(chunk_start_y + self.chunk_size) {
                        if let Some(entity) = self.get(&(x, y)) {
                            let distance = GridPosition::new(position.0, position.1)
                                .manhattan_distance_to(&GridPosition::new(x, y));
                            if distance <= 1 {
                                neighbors.push(entity);
                            }
                        }
                    }
                }
            }
        }
        
        neighbors
    }
    
    pub fn clear(&mut self) {
        self.cells.clear();
    }
    
    pub fn len(&self) -> usize {
        self.cells.len()
    }
    
    pub fn is_empty(&self) -> bool {
        self.cells.is_empty()
    }
    
    pub fn positions(&self) -> impl Iterator<Item = &(i32, i32)> {
        self.cells.keys()
    }
    
    pub fn entities(&self) -> impl Iterator<Item = &Entity> {
        self.cells.values()
    }
}

impl Default for SpatialGrid {
    fn default() -> Self {
        Self::new()
    }
}