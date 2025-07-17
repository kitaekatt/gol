use bevy::prelude::*;
use bevy::app::AppExit;
use crate::resources::{GridState, SimulationState, SimulationTimer, GameConfig};
use std::io::{self, BufRead};

#[derive(Event)]
pub struct InputEvent {
    pub key: char,
}

/// System for handling console input events
pub fn input_system(
    mut input_events: EventReader<InputEvent>,
    mut grid_state: ResMut<GridState>,
    mut simulation_state: ResMut<SimulationState>,
    mut simulation_timer: ResMut<SimulationTimer>,
    config: Res<GameConfig>,
    mut exit: EventWriter<AppExit>,
) {
    for event in input_events.read() {
        match event.key.to_ascii_lowercase() {
            'q' => {
                info!("Quit requested by user");
                exit.send(AppExit);
            }
            '\n' | '\r' => {
                info!("Manual generation advancement");
                simulation_timer.force_update();
            }
            ' ' => {
                if simulation_state.is_paused() {
                    simulation_state.resume();
                    info!("Simulation resumed");
                } else {
                    simulation_state.pause();
                    info!("Simulation paused");
                }
            }
            'r' => {
                // Clear current state
                grid_state.clear();
                simulation_state.reset();
                
                // Get config to restore default pattern
                let initial_positions = match config.initial_pattern.pattern_type.as_str() {
                    "embedded" => match config.initial_pattern.path.as_str() {
                        "glider" => crate::systems::game_of_life::generate_glider_pattern(10, 10),
                        "blinker" => crate::systems::game_of_life::generate_blinker_pattern(10, 10),
                        "block" => crate::systems::game_of_life::generate_block_pattern(10, 10),
                        "gosper_gun" => crate::systems::game_of_life::generate_gosper_gun_pattern(5, 5),
                        _ => crate::systems::game_of_life::generate_glider_pattern(10, 10),
                    },
                    "file" => crate::systems::game_of_life::generate_glider_pattern(10, 10),
                    _ => crate::systems::game_of_life::generate_glider_pattern(10, 10),
                };
                
                // Restore default pattern
                for position in initial_positions {
                    grid_state.add_cell(position);
                }
                
                info!("Simulation reset to default pattern");
            }
            'g' => {
                if let Some((center_x, center_y)) = grid_state.get_center() {
                    grid_state.set_pattern_glider(center_x as i32, center_y as i32);
                    info!("Spawned glider at center");
                } else {
                    grid_state.set_pattern_glider(10, 10);
                    info!("Spawned glider at (10, 10)");
                }
            }
            'b' => {
                if let Some((center_x, center_y)) = grid_state.get_center() {
                    grid_state.set_pattern_blinker(center_x as i32, center_y as i32);
                    info!("Spawned blinker at center");
                } else {
                    grid_state.set_pattern_blinker(10, 10);
                    info!("Spawned blinker at (10, 10)");
                }
            }
            'c' => {
                grid_state.clear();
                info!("Grid cleared");
            }
            'i' => {
                let live_cells = grid_state.count_live_cells();
                let generation = simulation_state.generation();
                info!("Generation: {}, Live cells: {}", generation, live_cells);
            }
            '+' | '=' => {
                simulation_timer.speed_up();
                info!("Simulation speed increased to {} steps/sec", simulation_timer.steps_per_second());
            }
            '-' => {
                simulation_timer.slow_down();
                info!("Simulation speed decreased to {} steps/sec", simulation_timer.steps_per_second());
            }
            _ => {} // Ignore other keys
        }
    }
}

/// System to read console input and send input events
/// Note: This is a placeholder - for real console input we'd need a separate thread
pub fn console_input_system(_input_events: EventWriter<InputEvent>) {
    // Placeholder - in a real implementation we'd need a separate thread
    // to read from stdin without blocking the main game loop
}