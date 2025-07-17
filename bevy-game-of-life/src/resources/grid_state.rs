use bevy::prelude::*;
use crate::components::grid::GridPosition;
use std::collections::HashSet;

#[derive(Resource, Debug, Default)]
pub struct GridState {
    pub live_cells: HashSet<(i32, i32)>,
    pub pending_births: HashSet<(i32, i32)>,
    pub pending_deaths: HashSet<(i32, i32)>,
    pub dirty: bool,
}

impl GridState {
    pub fn new() -> Self {
        Self {
            live_cells: HashSet::new(),
            pending_births: HashSet::new(),
            pending_deaths: HashSet::new(),
            dirty: false,
        }
    }
    
    pub fn from_positions(positions: Vec<(i32, i32)>) -> Self {
        let mut state = Self::new();
        state.live_cells = positions.into_iter().collect();
        state.dirty = true;
        state
    }
    
    pub fn add_cell(&mut self, position: (i32, i32)) {
        if self.live_cells.insert(position) {
            self.dirty = true;
        }
    }
    
    pub fn remove_cell(&mut self, position: &(i32, i32)) {
        if self.live_cells.remove(position) {
            self.dirty = true;
        }
    }
    
    pub fn toggle_cell(&mut self, position: (i32, i32)) {
        if self.live_cells.contains(&position) {
            self.remove_cell(&position);
        } else {
            self.add_cell(position);
        }
    }
    
    pub fn is_alive(&self, position: &(i32, i32)) -> bool {
        self.live_cells.contains(position)
    }
    
    pub fn get_live_cells(&self) -> &HashSet<(i32, i32)> {
        &self.live_cells
    }
    
    pub fn get_live_positions(&self) -> Vec<(i32, i32)> {
        self.live_cells.iter().cloned().collect()
    }
    
    pub fn get_live_grid_positions(&self) -> Vec<GridPosition> {
        self.live_cells.iter().map(|&pos| GridPosition::from_tuple(pos)).collect()
    }
    
    pub fn cell_count(&self) -> usize {
        self.live_cells.len()
    }
    
    pub fn is_empty(&self) -> bool {
        self.live_cells.is_empty()
    }
    
    pub fn clear(&mut self) {
        if !self.live_cells.is_empty() {
            self.live_cells.clear();
            self.pending_births.clear();
            self.pending_deaths.clear();
            self.dirty = true;
        }
    }
    
    pub fn get_bounds(&self) -> Option<(i32, i32, i32, i32)> {
        if self.live_cells.is_empty() {
            return None;
        }
        
        let mut min_x = i32::MAX;
        let mut max_x = i32::MIN;
        let mut min_y = i32::MAX;
        let mut max_y = i32::MIN;
        
        for &(x, y) in &self.live_cells {
            min_x = min_x.min(x);
            max_x = max_x.max(x);
            min_y = min_y.min(y);
            max_y = max_y.max(y);
        }
        
        Some((min_x, min_y, max_x, max_y))
    }
    
    pub fn get_center(&self) -> Option<(f32, f32)> {
        self.get_bounds().map(|(min_x, min_y, max_x, max_y)| {
            let center_x = (min_x + max_x) as f32 / 2.0;
            let center_y = (min_y + max_y) as f32 / 2.0;
            (center_x, center_y)
        })
    }
    
    // Pattern generation methods
    pub fn set_pattern_glider(&mut self, offset_x: i32, offset_y: i32) {
        let glider_positions = vec![
            (1 + offset_x, 0 + offset_y),
            (2 + offset_x, 1 + offset_y),
            (0 + offset_x, 2 + offset_y),
            (1 + offset_x, 2 + offset_y),
            (2 + offset_x, 2 + offset_y),
        ];
        
        for pos in glider_positions {
            self.add_cell(pos);
        }
    }
    
    pub fn set_pattern_blinker(&mut self, offset_x: i32, offset_y: i32) {
        let blinker_positions = vec![
            (1 + offset_x, 0 + offset_y),
            (1 + offset_x, 1 + offset_y),
            (1 + offset_x, 2 + offset_y),
        ];
        
        for pos in blinker_positions {
            self.add_cell(pos);
        }
    }
    
    pub fn set_pattern_block(&mut self, offset_x: i32, offset_y: i32) {
        let block_positions = vec![
            (0 + offset_x, 0 + offset_y),
            (0 + offset_x, 1 + offset_y),
            (1 + offset_x, 0 + offset_y),
            (1 + offset_x, 1 + offset_y),
        ];
        
        for pos in block_positions {
            self.add_cell(pos);
        }
    }
    
    pub fn set_pattern_toad(&mut self, offset_x: i32, offset_y: i32) {
        let toad_positions = vec![
            (1 + offset_x, 0 + offset_y),
            (2 + offset_x, 0 + offset_y),
            (3 + offset_x, 0 + offset_y),
            (0 + offset_x, 1 + offset_y),
            (1 + offset_x, 1 + offset_y),
            (2 + offset_x, 1 + offset_y),
        ];
        
        for pos in toad_positions {
            self.add_cell(pos);
        }
    }
    
    // Transition state management for smooth updates
    pub fn prepare_transition(&mut self, new_live_cells: HashSet<(i32, i32)>) {
        self.pending_births.clear();
        self.pending_deaths.clear();
        
        // Find cells that will be born
        for &pos in &new_live_cells {
            if !self.live_cells.contains(&pos) {
                self.pending_births.insert(pos);
            }
        }
        
        // Find cells that will die
        for &pos in &self.live_cells {
            if !new_live_cells.contains(&pos) {
                self.pending_deaths.insert(pos);
            }
        }
    }
    
    pub fn apply_transition(&mut self) {
        // Apply deaths
        for &pos in &self.pending_deaths {
            self.live_cells.remove(&pos);
        }
        
        // Apply births
        for &pos in &self.pending_births {
            self.live_cells.insert(pos);
        }
        
        // Clear pending changes
        self.pending_births.clear();
        self.pending_deaths.clear();
        self.dirty = true;
    }
    
    pub fn get_pending_births(&self) -> &HashSet<(i32, i32)> {
        &self.pending_births
    }
    
    pub fn get_pending_deaths(&self) -> &HashSet<(i32, i32)> {
        &self.pending_deaths
    }
    
    pub fn has_pending_changes(&self) -> bool {
        !self.pending_births.is_empty() || !self.pending_deaths.is_empty()
    }
    
    pub fn mark_clean(&mut self) {
        self.dirty = false;
    }
    
    pub fn is_dirty(&self) -> bool {
        self.dirty
    }
    
    pub fn set_from_cells(&mut self, cells: HashSet<(i32, i32)>) {
        self.live_cells = cells;
        self.pending_births.clear();
        self.pending_deaths.clear();
        self.dirty = true;
    }
    
    pub fn count_live_cells(&self) -> usize {
        self.live_cells.len()
    }
}