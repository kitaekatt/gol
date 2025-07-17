use crate::components::{CellState, GridPosition};
use crate::resources::{GameConfig, GridState, SimulationState, SimulationTimer};
use crate::plugins::{GameOfLifePlugin, ConfigPlugin};
use bevy::prelude::*;
use std::time::{Duration, Instant};

/// Clean interface for controlling the Game of Life simulation
/// Wraps Bevy ECS without exposing internal implementation details
pub struct SimulationController {
    app: App,
    last_update: Instant,
    target_fps: f64,
    frame_duration: Duration,
}

/// Public interface for simulation state
#[derive(Debug, Clone)]
pub struct SimulationSnapshot {
    pub generation: u64,
    pub live_cells: Vec<(i32, i32)>,
    pub population: usize,
    pub is_running: bool,
    pub grid_width: i32,
    pub grid_height: i32,
}

/// Simulation performance metrics
#[derive(Debug, Clone)]
pub struct PerformanceMetrics {
    pub fps: f64,
    pub generation: u64,
    pub cells_processed: usize,
    pub update_time_ms: f64,
}

impl SimulationController {
    /// Create a new simulation controller with default configuration
    pub fn new() -> Self {
        Self::with_config(GameConfig::default())
    }
    
    /// Create a new simulation controller with custom configuration
    pub fn with_config(config: GameConfig) -> Self {
        let mut app = App::new();
        
        // Add minimal Bevy plugins (headless)
        app.add_plugins(MinimalPlugins);
        
        // Add our custom plugins
        app.add_plugins((
            ConfigPlugin::default(),
            GameOfLifePlugin,
        ));
        
        // Override with custom config
        app.insert_resource(config.clone());
        
        let target_fps = 60.0; // Default to 60 FPS
        let frame_duration = Duration::from_secs_f64(1.0 / target_fps);
        
        Self {
            app,
            last_update: Instant::now(),
            target_fps,
            frame_duration,
        }
    }
    
    /// Start the simulation
    pub fn start(&mut self) {
        if let Some(mut sim_state) = self.app.world.get_resource_mut::<SimulationState>() {
            sim_state.start();
        }
        if let Some(mut timer) = self.app.world.get_resource_mut::<SimulationTimer>() {
            timer.unpause();
        }
    }
    
    /// Pause the simulation
    pub fn pause(&mut self) {
        if let Some(mut sim_state) = self.app.world.get_resource_mut::<SimulationState>() {
            sim_state.pause();
        }
        if let Some(mut timer) = self.app.world.get_resource_mut::<SimulationTimer>() {
            timer.pause();
        }
    }
    
    /// Step the simulation forward by one generation
    pub fn step(&mut self) {
        // Temporarily unpause, update once, then pause again
        let was_running = self.is_running();
        
        if !was_running {
            self.start();
        }
        
        // Force the timer to trigger immediately for one step
        if let Some(mut timer) = self.app.world.get_resource_mut::<SimulationTimer>() {
            timer.force_update();
        }
        
        // Force one simulation step
        self.update_once();
        
        if !was_running {
            self.pause();
        }
    }
    
    /// Reset the simulation to initial state
    pub fn reset(&mut self) {
        // Reset simulation state
        if let Some(mut sim_state) = self.app.world.get_resource_mut::<SimulationState>() {
            sim_state.reset();
        }
        
        // Reset timer
        if let Some(mut timer) = self.app.world.get_resource_mut::<SimulationTimer>() {
            timer.reset();
        }
        
        // Clear grid and regenerate initial pattern
        if let Some(mut grid_state) = self.app.world.get_resource_mut::<GridState>() {
            grid_state.clear();
        }
        
        // Despawn all existing cell entities
        let mut entities_to_remove = Vec::new();
        let mut cell_query = self.app.world.query_filtered::<Entity, With<CellState>>();
        for entity in cell_query.iter(&self.app.world) {
            entities_to_remove.push(entity);
        }
        
        for entity in entities_to_remove {
            self.app.world.despawn(entity);
        }
        
        // Run initialization systems to create initial pattern
        self.app.update();
    }
    
    /// Load a pattern from configuration
    pub fn load_pattern(&mut self, pattern_name: &str) {
        use crate::systems::game_of_life::*;
        
        // Clear existing cells first
        self.set_cells(&[]);
        
        // Generate the pattern directly
        let pattern_cells = match pattern_name {
            "glider" => generate_glider_pattern(10, 10),
            "blinker" => generate_blinker_pattern(10, 10),
            "block" => generate_block_pattern(10, 10),
            "gosper_gun" => generate_gosper_gun_pattern(5, 5),
            _ => generate_glider_pattern(10, 10), // Default to glider
        };
        
        // Set the new pattern
        self.set_cells(&pattern_cells);
        
        // Update config to reflect the change
        if let Some(mut config) = self.app.world.get_resource_mut::<GameConfig>() {
            config.initial_pattern.path = pattern_name.to_string();
        }
    }
    
    /// Set custom live cells directly
    pub fn set_cells(&mut self, cells: &[(i32, i32)]) {
        // Clear existing cells
        let mut entities_to_remove = Vec::new();
        let mut cell_query = self.app.world.query_filtered::<Entity, With<CellState>>();
        for entity in cell_query.iter(&self.app.world) {
            entities_to_remove.push(entity);
        }
        
        for entity in entities_to_remove {
            self.app.world.despawn(entity);
        }
        
        // Clear grid state
        if let Some(mut grid_state) = self.app.world.get_resource_mut::<GridState>() {
            grid_state.clear();
        }
        
        // Add new cells
        for &(x, y) in cells {
            self.app.world.spawn((
                CellState::new(true),
                GridPosition::new(x, y),
            ));
            
            if let Some(mut grid_state) = self.app.world.get_resource_mut::<GridState>() {
                grid_state.add_cell((x, y));
            }
        }
    }
    
    /// Get current simulation state snapshot
    pub fn get_state(&mut self) -> SimulationSnapshot {
        let generation = self.app.world.get_resource::<SimulationState>()
            .map(|s| s.get_generation())
            .unwrap_or(0);
            
        let is_running = self.app.world.get_resource::<SimulationState>()
            .map(|s| s.is_running())
            .unwrap_or(false);
            
        let config = self.app.world.get_resource::<GameConfig>()
            .cloned()
            .unwrap_or_default();
            
        let grid_state = self.app.world.get_resource::<GridState>();
        
        let live_cells: Vec<(i32, i32)> = if let Some(grid_state) = grid_state {
            grid_state.get_live_cells().iter().cloned().collect()
        } else {
            // Fallback: query entities directly
            let mut cell_query = self.app.world.query_filtered::<&GridPosition, With<CellState>>();
            cell_query.iter(&self.app.world)
                .map(|pos| pos.to_tuple())
                .collect()
        };
        
        let population = live_cells.len();
        
        SimulationSnapshot {
            generation,
            live_cells,
            population,
            is_running,
            grid_width: config.grid.width,
            grid_height: config.grid.height,
        }
    }
    
    /// Get performance metrics
    pub fn get_performance(&mut self) -> PerformanceMetrics {
        let sim_state = self.app.world.get_resource::<SimulationState>();
        
        let (generation, fps) = sim_state
            .and_then(|s| s.get_performance_info())
            .unwrap_or((0, 0.0));
            
        let population = self.get_state().population;
        
        PerformanceMetrics {
            fps,
            generation,
            cells_processed: population,
            update_time_ms: 1000.0 / fps.max(0.1), // Avoid division by zero
        }
    }
    
    /// Check if simulation is currently running
    pub fn is_running(&self) -> bool {
        self.app.world.get_resource::<SimulationState>()
            .map(|s| s.is_running())
            .unwrap_or(false)
    }
    
    /// Set target frame rate
    pub fn set_fps(&mut self, fps: f64) {
        self.target_fps = fps.max(1.0).min(1000.0); // Clamp between 1-1000 FPS
        self.frame_duration = Duration::from_secs_f64(1.0 / self.target_fps);
        
        // Update simulation timer
        if let Some(mut timer) = self.app.world.get_resource_mut::<SimulationTimer>() {
            *timer = SimulationTimer::new(self.target_fps as u32);
        }
    }
    
    /// Update simulation (should be called regularly)
    pub fn update(&mut self) {
        let now = Instant::now();
        let elapsed = now.duration_since(self.last_update);
        
        if elapsed >= self.frame_duration {
            self.update_once();
            self.last_update = now;
        }
    }
    
    /// Force a single simulation update regardless of timing
    fn update_once(&mut self) {
        self.app.update();
    }
    
    /// Get grid bounds for rendering
    pub fn get_grid_bounds(&mut self) -> (i32, i32, i32, i32) {
        let config = self.app.world.get_resource::<GameConfig>()
            .cloned()
            .unwrap_or_default();
            
        if config.grid.wrap_edges {
            // For wrapped grids, use configured bounds
            (0, 0, config.grid.width, config.grid.height)
        } else {
            // For unwrapped grids, find actual bounds of live cells
            let state = self.get_state();
            if state.live_cells.is_empty() {
                (0, 0, config.grid.width.min(50), config.grid.height.min(50))
            } else {
                let min_x = state.live_cells.iter().map(|(x, _)| *x).min().unwrap_or(0);
                let max_x = state.live_cells.iter().map(|(x, _)| *x).max().unwrap_or(0);
                let min_y = state.live_cells.iter().map(|(_, y)| *y).min().unwrap_or(0);
                let max_y = state.live_cells.iter().map(|(_, y)| *y).max().unwrap_or(0);
                
                // Add some padding
                (min_x - 5, min_y - 5, max_x + 5, max_y + 5)
            }
        }
    }
}

impl Default for SimulationController {
    fn default() -> Self {
        Self::new()
    }
}