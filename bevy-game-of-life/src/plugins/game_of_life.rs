use bevy::prelude::*;
use crate::components::SpatialGrid;
use crate::resources::{GameConfig, GridState, SimulationState, SimulationTimer};
use crate::systems::{add_game_of_life_systems};

/// Main plugin for the Game of Life implementation
pub struct GameOfLifePlugin;

impl Plugin for GameOfLifePlugin {
    fn build(&self, app: &mut App) {
        // Add resources
        app.insert_resource(GameConfig::default())
            .insert_resource(GridState::new())
            .insert_resource(SpatialGrid::new())
            .insert_resource(SimulationState::new());
        
        // Initialize simulation timer from config
        let config = app.world.get_resource::<GameConfig>().unwrap();
        let timer = SimulationTimer::from_config(config);
        app.insert_resource(timer);
        
        // Add all Game of Life systems
        add_game_of_life_systems(app);
        
        info!("Game of Life plugin initialized");
    }
}


/// Configuration plugin for loading/managing settings
pub struct ConfigPlugin {
    pub config_path: Option<String>,
}

impl ConfigPlugin {
    pub fn new() -> Self {
        Self { config_path: None }
    }
    
    pub fn with_config_path(config_path: String) -> Self {
        Self { config_path: Some(config_path) }
    }
}

impl Default for ConfigPlugin {
    fn default() -> Self {
        Self::new()
    }
}

impl Plugin for ConfigPlugin {
    fn build(&self, app: &mut App) {
        // Load config from file if path is provided
        if let Some(path) = &self.config_path {
            match GameConfig::load_from_file(path) {
                Ok(config) => {
                    info!("Loaded config from: {}", path);
                    app.insert_resource(config);
                }
                Err(e) => {
                    warn!("Failed to load config from {}: {}. Using default.", path, e);
                    app.insert_resource(GameConfig::default());
                }
            }
        } else {
            app.insert_resource(GameConfig::default());
        }
        
        // Add config management systems
        app.add_systems(Update, (
            hot_reload_config_system,
            save_config_on_change_system,
        ));
    }
}

/// System to hot-reload configuration (placeholder)
fn hot_reload_config_system(
    // keyboard_input: Res<Input<KeyCode>>,
    // mut config: ResMut<GameConfig>,
) {
    // TODO: Implement hot-reload on F5 or file change
}

/// System to save configuration when it changes (placeholder)
fn save_config_on_change_system(
    // config: Res<GameConfig>,
    // mut last_config: Local<Option<GameConfig>>,
) {
    // TODO: Implement automatic config saving
}

/// Development tools plugin with debug features
pub struct DebugPlugin;

impl Plugin for DebugPlugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Update, (
            debug_input_system,
            performance_monitoring_system,
        ));
    }
}

/// System for debug commands (headless mode)
fn debug_input_system(
    mut grid_state: ResMut<GridState>,
    _simulation_state: Res<SimulationState>,
) {
    // Auto-spawn initial pattern for testing
    static mut SPAWNED: bool = false;
    unsafe {
        if !SPAWNED {
            // Spawn glider at a fixed position since grid starts empty
            grid_state.set_pattern_glider(10, 10);
            info!("Auto-spawned glider at (10, 10) for testing");
            SPAWNED = true;
        }
    }
}

/// System for performance monitoring
fn performance_monitoring_system(
    simulation_state: Res<SimulationState>,
    mut last_report: Local<Option<std::time::Instant>>,
) {
    let now = std::time::Instant::now();
    let should_report = last_report.map_or(true, |t| now.duration_since(t) >= std::time::Duration::from_secs(5));
    if should_report {
        if let Some((generation, fps)) = simulation_state.get_performance_info() {
            info!("Performance: Gen {}, {:.1} FPS", generation, fps);
        }
        *last_report = Some(now);
    }
}