use bevy::prelude::*;
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;
use anyhow::{Context, Result};

#[derive(Resource, Debug, Clone, Serialize, Deserialize)]
pub struct GameConfig {
    pub grid: GridConfig,
    pub simulation: SimulationConfig,
    pub rendering: RenderingConfig,
    pub initial_pattern: PatternConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GridConfig {
    pub width: i32,
    pub height: i32,
    pub wrap_edges: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SimulationConfig {
    pub steps_per_second: u32,
    pub auto_start: bool,
    pub max_generations: Option<u64>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RenderingConfig {
    pub cell_size: f32,
    pub grid_lines: bool,
    pub color_scheme: String,
    pub smooth_transitions: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PatternConfig {
    #[serde(rename = "type")]
    pub pattern_type: String,
    pub path: String,
}

impl Default for GameConfig {
    fn default() -> Self {
        Self {
            grid: GridConfig::default(),
            simulation: SimulationConfig::default(),
            rendering: RenderingConfig::default(),
            initial_pattern: PatternConfig::default(),
        }
    }
}

impl Default for GridConfig {
    fn default() -> Self {
        Self {
            width: 100,
            height: 100,
            wrap_edges: false,
        }
    }
}

impl Default for SimulationConfig {
    fn default() -> Self {
        Self {
            steps_per_second: 10,
            auto_start: true,
            max_generations: None,
        }
    }
}

impl Default for RenderingConfig {
    fn default() -> Self {
        Self {
            cell_size: 8.0,
            grid_lines: true,
            color_scheme: "classic".to_string(),
            smooth_transitions: true,
        }
    }
}

impl Default for PatternConfig {
    fn default() -> Self {
        Self {
            pattern_type: "embedded".to_string(),
            path: "glider".to_string(),
        }
    }
}

impl GameConfig {
    pub fn load_from_file<P: AsRef<Path>>(path: P) -> Result<Self> {
        let content = fs::read_to_string(path.as_ref())
            .with_context(|| format!("Failed to read config file: {}", path.as_ref().display()))?;
        
        let config: GameConfig = serde_json::from_str(&content)
            .with_context(|| "Failed to parse config JSON")?;
        
        config.validate()?;
        Ok(config)
    }
    
    pub fn save_to_file<P: AsRef<Path>>(&self, path: P) -> Result<()> {
        self.validate()?;
        
        let content = serde_json::to_string_pretty(self)
            .with_context(|| "Failed to serialize config")?;
        
        fs::write(path.as_ref(), content)
            .with_context(|| format!("Failed to write config file: {}", path.as_ref().display()))?;
        
        Ok(())
    }
    
    pub fn validate(&self) -> Result<()> {
        // Validate grid configuration
        if self.grid.width <= 0 || self.grid.height <= 0 {
            return Err(anyhow::anyhow!("Grid dimensions must be positive"));
        }
        
        if self.grid.width > 10000 || self.grid.height > 10000 {
            return Err(anyhow::anyhow!("Grid dimensions too large (max 10000x10000)"));
        }
        
        // Validate simulation configuration
        if self.simulation.steps_per_second == 0 {
            return Err(anyhow::anyhow!("Steps per second must be positive"));
        }
        
        if self.simulation.steps_per_second > 1000 {
            return Err(anyhow::anyhow!("Steps per second too high (max 1000)"));
        }
        
        // Validate rendering configuration
        if self.rendering.cell_size <= 0.0 {
            return Err(anyhow::anyhow!("Cell size must be positive"));
        }
        
        if self.rendering.cell_size > 100.0 {
            return Err(anyhow::anyhow!("Cell size too large (max 100.0)"));
        }
        
        // Validate pattern configuration
        if self.initial_pattern.pattern_type.is_empty() {
            return Err(anyhow::anyhow!("Pattern type cannot be empty"));
        }
        
        if !["file", "embedded"].contains(&self.initial_pattern.pattern_type.as_str()) {
            return Err(anyhow::anyhow!("Pattern type must be 'file' or 'embedded'"));
        }
        
        Ok(())
    }
    
    pub fn get_step_duration(&self) -> std::time::Duration {
        std::time::Duration::from_secs_f64(1.0 / self.simulation.steps_per_second as f64)
    }
}

#[derive(Resource, Debug)]
pub struct SimulationTimer {
    timer: Timer,
    steps_per_second: u32,
    force_step: bool,
}

impl SimulationTimer {
    pub fn new(steps_per_second: u32) -> Self {
        let duration = std::time::Duration::from_secs_f64(1.0 / steps_per_second as f64);
        Self {
            timer: Timer::new(duration, TimerMode::Repeating),
            steps_per_second,
            force_step: false,
        }
    }
    
    pub fn from_config(config: &GameConfig) -> Self {
        Self::new(config.simulation.steps_per_second)
    }
    
    pub fn tick(&mut self, delta: std::time::Duration) -> bool {
        let timer_finished = self.timer.tick(delta).just_finished();
        let should_step = timer_finished || self.force_step;
        
        if self.force_step {
            self.force_step = false; // Reset the flag
        }
        
        should_step
    }
    
    pub fn set_steps_per_second(&mut self, steps_per_second: u32) {
        self.steps_per_second = steps_per_second;
        let duration = std::time::Duration::from_secs_f64(1.0 / steps_per_second as f64);
        self.timer.set_duration(duration);
    }
    
    pub fn steps_per_second(&self) -> u32 {
        self.steps_per_second
    }
    
    pub fn pause(&mut self) {
        self.timer.pause();
    }
    
    pub fn unpause(&mut self) {
        self.timer.unpause();
    }
    
    pub fn is_paused(&self) -> bool {
        self.timer.paused()
    }
    
    pub fn force_update(&mut self) {
        // Set flag to force next step
        self.force_step = true;
    }
    
    pub fn reset(&mut self) {
        self.timer.reset();
    }
    
    pub fn elapsed(&self) -> std::time::Duration {
        self.timer.elapsed()
    }
    
    pub fn remaining(&self) -> std::time::Duration {
        self.timer.remaining()
    }
    
    pub fn speed_up(&mut self) {
        let new_speed = (self.steps_per_second * 2).min(100);
        self.set_steps_per_second(new_speed);
    }
    
    pub fn slow_down(&mut self) {
        let new_speed = (self.steps_per_second / 2).max(1);
        self.set_steps_per_second(new_speed);
    }
}

#[derive(Resource, Debug, Default)]
pub struct SimulationState {
    pub generation: u64,
    pub running: bool,
    pub cell_count: usize,
    pub last_step_duration: Option<std::time::Duration>,
}

impl SimulationState {
    pub fn new() -> Self {
        Self {
            generation: 0,
            running: false,
            cell_count: 0,
            last_step_duration: None,
        }
    }
    
    pub fn start(&mut self) {
        self.running = true;
    }
    
    pub fn pause(&mut self) {
        self.running = false;
    }
    
    pub fn toggle(&mut self) {
        self.running = !self.running;
    }
    
    pub fn reset(&mut self) {
        self.generation = 0;
        self.running = false;
        self.cell_count = 0;
        self.last_step_duration = None;
    }
    
    pub fn advance_generation(&mut self, cell_count: usize, step_duration: std::time::Duration) {
        self.generation += 1;
        self.cell_count = cell_count;
        self.last_step_duration = Some(step_duration);
    }
    
    pub fn is_running(&self) -> bool {
        self.running
    }
    
    pub fn get_generation(&self) -> u64 {
        self.generation
    }
    
    pub fn get_cell_count(&self) -> usize {
        self.cell_count
    }
    
    pub fn get_performance_info(&self) -> Option<(u64, f64)> {
        self.last_step_duration.map(|duration| {
            let fps = 1.0 / duration.as_secs_f64();
            (self.generation, fps)
        })
    }
    
    pub fn is_paused(&self) -> bool {
        !self.running
    }
    
    pub fn resume(&mut self) {
        self.running = true;
    }
    
    pub fn generation(&self) -> u64 {
        self.generation
    }
}