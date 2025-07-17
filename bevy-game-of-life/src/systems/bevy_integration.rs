use bevy::prelude::*;
use crate::components::{CellState, GridPosition, NeighborCount, SpatialGrid};
use crate::resources::{GameConfig, GridState, SimulationState, SimulationTimer};
use crate::systems::game_of_life::*;
use crate::systems::input::{input_system, console_input_system, InputEvent};
use std::collections::HashSet;

// System to update neighbor counts for all cells
pub fn update_neighbor_counts_system(
    mut cell_query: Query<(&GridPosition, &mut NeighborCount), With<CellState>>,
    spatial_grid: Res<SpatialGrid>,
    config: Res<GameConfig>,
) {
    for (position, mut neighbor_count) in cell_query.iter_mut() {
        if neighbor_count.is_dirty() {
            let live_positions: HashSet<(i32, i32)> = spatial_grid
                .positions()
                .cloned()
                .collect();
            
            let count = if config.grid.wrap_edges {
                count_live_neighbors_with_wrapping(
                    position.to_tuple(),
                    &live_positions,
                    config.grid.width,
                    config.grid.height,
                )
            } else {
                count_live_neighbors(position.to_tuple(), &live_positions, false)
            };
            
            neighbor_count.set_count(count);
        }
    }
}

// System to apply Game of Life rules and determine next generation
pub fn apply_game_of_life_system(
    cell_query: Query<(Entity, &GridPosition, &CellState, &NeighborCount)>,
    mut grid_state: ResMut<GridState>,
    _spatial_grid: ResMut<SpatialGrid>,
    config: Res<GameConfig>,
    _timer: Res<SimulationTimer>,
    simulation_state: Res<SimulationState>,
    mut last_generation: Local<u64>,
) {
    // Check if we should apply rules based on generation change
    let current_generation = simulation_state.get_generation();
    let should_apply = simulation_state.is_running() || current_generation > *last_generation;
    
    if !should_apply {
        return;
    }
    
    *last_generation = current_generation;
    
    // Collect current live cells
    let current_live_cells: Vec<(i32, i32)> = cell_query
        .iter()
        .filter(|(_, _, cell_state, _)| cell_state.is_alive())
        .map(|(_, position, _, _)| position.to_tuple())
        .collect();
    
    // Apply Game of Life rules
    let next_generation = if config.grid.wrap_edges {
        apply_game_of_life_rules(
            &current_live_cells,
            true,
            Some(config.grid.width),
            Some(config.grid.height),
        )
    } else {
        apply_game_of_life_rules(&current_live_cells, false, None, None)
    };
    
    // Update grid state with new generation
    let next_live_set: HashSet<(i32, i32)> = next_generation.into_iter().collect();
    grid_state.prepare_transition(next_live_set);
}

// System to spawn new cell entities for births
pub fn spawn_new_cells_system(
    mut commands: Commands,
    grid_state: Res<GridState>,
    mut spatial_grid: ResMut<SpatialGrid>,
    _config: Res<GameConfig>,
) {
    for &position in grid_state.get_pending_births() {
        let entity = commands.spawn((
            CellState::new(true),
            GridPosition::from_tuple(position),
            NeighborCount::new(),
        )).id();
        
        // Update spatial grid
        spatial_grid.insert(position, entity);
    }
}

// System to despawn cell entities for deaths
pub fn despawn_dead_cells_system(
    mut commands: Commands,
    cell_query: Query<(Entity, &GridPosition), With<CellState>>,
    grid_state: Res<GridState>,
    mut spatial_grid: ResMut<SpatialGrid>,
) {
    for &death_position in grid_state.get_pending_deaths() {
        // Find entity at this position and despawn it
        for (entity, position) in cell_query.iter() {
            if position.to_tuple() == death_position {
                commands.entity(entity).despawn();
                spatial_grid.remove(&death_position);
                break;
            }
        }
    }
}

// System to finalize the generation transition
pub fn finalize_generation_system(
    mut grid_state: ResMut<GridState>,
    mut simulation_state: ResMut<SimulationState>,
    cell_query: Query<&CellState>,
    _time: Res<Time>,
) {
    if grid_state.has_pending_changes() {
        let start_time = std::time::Instant::now();
        
        // Apply the transition
        grid_state.apply_transition();
        
        // Update simulation state
        let live_count = cell_query.iter().filter(|cell| cell.is_alive()).count();
        let step_duration = start_time.elapsed();
        simulation_state.advance_generation(live_count, step_duration);
    }
}

// System to handle simulation timing
pub fn simulation_timer_system(
    mut timer: ResMut<SimulationTimer>,
    time: Res<Time>,
    mut cell_query: Query<&mut NeighborCount>,
) {
    if timer.tick(time.delta()) {
        // Mark all neighbor counts as dirty for recalculation
        for mut neighbor_count in cell_query.iter_mut() {
            neighbor_count.mark_dirty();
        }
    }
}

// System to synchronize grid state with ECS entities
pub fn sync_grid_state_system(
    cell_query: Query<(&GridPosition, &CellState), Changed<CellState>>,
    mut grid_state: ResMut<GridState>,
) {
    let mut _any_changes = false;
    
    for (position, cell_state) in cell_query.iter() {
        let pos_tuple = position.to_tuple();
        
        if cell_state.is_alive() {
            if !grid_state.is_alive(&pos_tuple) {
                grid_state.add_cell(pos_tuple);
                _any_changes = true;
            }
        } else {
            if grid_state.is_alive(&pos_tuple) {
                grid_state.remove_cell(&pos_tuple);
                _any_changes = true;
            }
        }
    }
}

// System to update spatial grid when entities move or change
pub fn update_spatial_grid_system(
    entity_query: Query<(Entity, &GridPosition), (With<CellState>, Changed<GridPosition>)>,
    mut spatial_grid: ResMut<SpatialGrid>,
) {
    for (entity, position) in entity_query.iter() {
        spatial_grid.insert(position.to_tuple(), entity);
    }
}

// System to handle simulation control (headless - auto-run)
pub fn simulation_control_system(
    _simulation_state: Res<SimulationState>,
) {
    // In headless mode, simulation runs automatically
    // This system is kept for consistency but does nothing
}

// System to initialize the game world with a pattern
pub fn initialize_pattern_system(
    mut commands: Commands,
    mut grid_state: ResMut<GridState>,
    mut spatial_grid: ResMut<SpatialGrid>,
    config: Res<GameConfig>,
    mut initialization_done: Local<bool>,
) {
    if *initialization_done {
        return;
    }
    
    // Clear any existing state
    grid_state.clear();
    spatial_grid.clear();
    
    // Load initial pattern based on config
    let initial_positions = match config.initial_pattern.pattern_type.as_str() {
        "embedded" => match config.initial_pattern.path.as_str() {
            "glider" => generate_glider_pattern(10, 10),
            "blinker" => generate_blinker_pattern(10, 10),
            "block" => generate_block_pattern(10, 10),
            "gosper_gun" => generate_gosper_gun_pattern(5, 5),
            _ => generate_glider_pattern(10, 10), // Default to glider
        },
        "file" => {
            // TODO: Load from file
            generate_glider_pattern(10, 10)
        },
        _ => generate_glider_pattern(10, 10),
    };
    
    // Spawn entities for initial pattern
    for position in initial_positions {
        let entity = commands.spawn((
            CellState::new(true),
            GridPosition::from_tuple(position),
            NeighborCount::new(),
        )).id();
        
        grid_state.add_cell(position);
        spatial_grid.insert(position, entity);
    }
    
    *initialization_done = true;
}

// System to update cell ages
pub fn update_cell_ages_system(
    mut cell_query: Query<&mut CellState>,
    simulation_state: Res<SimulationState>,
    mut last_generation: Local<u64>,
) {
    if simulation_state.get_generation() > *last_generation {
        for mut cell_state in cell_query.iter_mut() {
            if cell_state.is_alive() {
                cell_state.increment_age();
            }
        }
        *last_generation = simulation_state.get_generation();
    }
}

// Debug system to print simulation statistics
pub fn debug_statistics_system(
    simulation_state: Res<SimulationState>,
    grid_state: Res<GridState>,
    mut last_stats_time: Local<Option<std::time::Instant>>,
) {
    let now = std::time::Instant::now();
    let should_log = last_stats_time.map_or(true, |t| now.duration_since(t) >= std::time::Duration::from_secs(1));
    if should_log {
        if let Some((generation, fps)) = simulation_state.get_performance_info() {
            info!(
                "Generation: {}, Cells: {}, FPS: {:.2}",
                generation,
                grid_state.cell_count(),
                fps
            );
        }
        *last_stats_time = Some(now);
    }
}

// System set for organizing Game of Life systems
#[derive(SystemSet, Debug, Hash, PartialEq, Eq, Clone)]
pub enum GameOfLifeSystemSet {
    Input,
    Logic,
    Spawn,
    Cleanup,
    Debug,
}

// Helper function to add all Game of Life systems to an app
pub fn add_game_of_life_systems(app: &mut App) {
    app.add_event::<InputEvent>()
    .configure_sets(
        Update,
        (
            GameOfLifeSystemSet::Input,
            GameOfLifeSystemSet::Logic,
            GameOfLifeSystemSet::Spawn,
            GameOfLifeSystemSet::Cleanup,
            GameOfLifeSystemSet::Debug,
        ).chain()
    )
    .add_systems(
        Update,
        (
            (simulation_control_system, console_input_system, input_system).in_set(GameOfLifeSystemSet::Input),
            (
                simulation_timer_system,
                update_neighbor_counts_system,
                apply_game_of_life_system,
            ).in_set(GameOfLifeSystemSet::Logic),
            (
                spawn_new_cells_system,
                despawn_dead_cells_system,
            ).in_set(GameOfLifeSystemSet::Spawn),
            (
                finalize_generation_system,
                sync_grid_state_system,
                update_spatial_grid_system,
                update_cell_ages_system,
            ).in_set(GameOfLifeSystemSet::Cleanup),
            debug_statistics_system.in_set(GameOfLifeSystemSet::Debug),
        ),
    )
    .add_systems(Startup, initialize_pattern_system);
}