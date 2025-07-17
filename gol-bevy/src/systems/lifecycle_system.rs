use bevy::prelude::*;
use crate::components::{Position, CellState};
use crate::resources::SimulationState;

pub fn lifecycle_system(
    mut commands: Commands,
    mut query: Query<(Entity, &Position, &mut CellState)>,
    mut simulation_state: ResMut<SimulationState>,
) {
    let mut entities_to_despawn = Vec::new();
    let mut entities_to_birth = Vec::new();
    
    // Process all cells and apply Conway's Game of Life rules
    for (entity, position, mut cell_state) in query.iter_mut() {
        if cell_state.alive {
            // Living cell logic
            if cell_state.should_survive() {
                // Cell survives to next generation
                cell_state.generation += 1;
                cell_state.neighbor_count = 0; // Reset for next cycle
            } else {
                // Cell dies
                entities_to_despawn.push(entity);
            }
        } else {
            // Dead cell logic
            if cell_state.should_be_born() {
                // Cell comes to life
                entities_to_birth.push((*position, entity));
            } else {
                // Dead cell remains dead, remove ghost cell
                entities_to_despawn.push(entity);
            }
        }
    }
    
    // Despawn dead cells
    for entity in entities_to_despawn {
        commands.entity(entity).despawn();
    }
    
    // Birth new cells
    for (position, entity) in entities_to_birth {
        // Update the existing ghost cell to be alive
        if let Some(mut entity_commands) = commands.get_entity(entity) {
            entity_commands.insert(CellState {
                alive: true,
                generation: simulation_state.generation + 1,
                neighbor_count: 0,
            });
        }
    }
    
    // Increment generation
    simulation_state.generation += 1;
}

pub fn cell_lifecycle_system(
    mut commands: Commands,
    mut query: Query<(Entity, &Position, &mut CellState)>,
    mut simulation_state: ResMut<SimulationState>,
) {
    let mut cells_to_process = Vec::new();
    
    // Collect all cells that need processing
    for (entity, position, cell_state) in query.iter() {
        cells_to_process.push((entity, *position, *cell_state));
    }
    
    // Process each cell according to Game of Life rules
    for (entity, position, cell_state) in cells_to_process {
        if cell_state.alive {
            // Living cell rules
            match cell_state.neighbor_count {
                2 | 3 => {
                    // Cell survives
                    if let Some(mut entity_commands) = commands.get_entity(entity) {
                        entity_commands.insert(CellState {
                            alive: true,
                            generation: cell_state.generation + 1,
                            neighbor_count: 0, // Reset for next cycle
                        });
                    }
                },
                _ => {
                    // Cell dies (underpopulation or overpopulation)
                    commands.entity(entity).despawn();
                }
            }
        } else {
            // Dead cell rules
            if cell_state.neighbor_count == 3 {
                // Cell is born
                commands.entity(entity).insert(CellState {
                    alive: true,
                    generation: simulation_state.generation + 1,
                    neighbor_count: 0,
                });
            } else {
                // Dead cell remains dead, remove ghost cell
                commands.entity(entity).despawn();
            }
        }
    }
    
    // Increment generation
    simulation_state.generation += 1;
}

pub fn cleanup_system(
    mut commands: Commands,
    query: Query<(Entity, &CellState)>,
) {
    // Remove dead ghost cells that weren't born
    for (entity, cell_state) in query.iter() {
        if !cell_state.alive && cell_state.neighbor_count != 3 {
            commands.entity(entity).despawn();
        }
    }
}