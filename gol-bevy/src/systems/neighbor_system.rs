use bevy::prelude::*;
use crate::components::{Position, CellState};
use std::collections::HashMap;

pub fn neighbor_calculation_system(
    mut query: Query<(&Position, &mut CellState)>,
    live_cells: Query<&Position, With<CellState>>,
) {
    // Create a spatial index of all live cells for efficient neighbor lookups
    let live_positions: HashMap<(i32, i32), ()> = live_cells
        .iter()
        .map(|pos| ((pos.x, pos.y), ()))
        .collect();
    
    // Calculate potential cells that need neighbor counting
    // This includes all live cells and their neighbors (sparse representation)
    let mut potential_cells: HashMap<(i32, i32), u8> = HashMap::new();
    
    // For each live cell, count its contribution to neighbor counts
    for live_pos in live_cells.iter() {
        let neighbors = live_pos.neighbors();
        
        // Each live cell contributes +1 to its neighbors' count
        for neighbor_pos in neighbors {
            *potential_cells.entry((neighbor_pos.x, neighbor_pos.y)).or_insert(0) += 1;
        }
    }
    
    // Update neighbor counts for all live cells
    for (position, mut cell_state) in query.iter_mut() {
        let neighbor_count = potential_cells
            .get(&(position.x, position.y))
            .copied()
            .unwrap_or(0);
        
        cell_state.neighbor_count = neighbor_count;
    }
}

pub fn neighbor_counting_system(
    mut commands: Commands,
    mut query: Query<(Entity, &Position, &mut CellState)>,
    live_cells: Query<&Position, With<CellState>>,
) {
    // Create a spatial index of all live cells for efficient neighbor lookups
    let live_positions: HashMap<(i32, i32), ()> = live_cells
        .iter()
        .map(|pos| ((pos.x, pos.y), ()))
        .collect();
    
    // Calculate potential cells that need neighbor counting
    // This includes all live cells and their neighbors (sparse representation)
    let mut potential_cells: HashMap<(i32, i32), u8> = HashMap::new();
    
    // For each live cell, count its contribution to neighbor counts
    for live_pos in live_cells.iter() {
        let neighbors = live_pos.neighbors();
        
        // Each live cell contributes +1 to its neighbors' count
        for neighbor_pos in neighbors {
            *potential_cells.entry((neighbor_pos.x, neighbor_pos.y)).or_insert(0) += 1;
        }
    }
    
    // Update neighbor counts for existing live cells
    for (entity, position, mut cell_state) in query.iter_mut() {
        let neighbor_count = potential_cells
            .get(&(position.x, position.y))
            .copied()
            .unwrap_or(0);
        
        cell_state.neighbor_count = neighbor_count;
    }
    
    // Create ghost cells for positions that might birth new cells
    // These are dead cells with exactly 3 neighbors
    for ((x, y), neighbor_count) in potential_cells.iter() {
        if *neighbor_count == 3 && !live_positions.contains_key(&(*x, *y)) {
            // This is a potential birth position
            commands.spawn((
                Position::new(*x, *y),
                CellState {
                    alive: false,
                    generation: 0,
                    neighbor_count: *neighbor_count,
                },
            ));
        }
    }
}