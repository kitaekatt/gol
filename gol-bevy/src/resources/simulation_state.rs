use bevy::prelude::*;
use uuid::Uuid;
use std::collections::HashMap;

#[derive(Resource, Clone, Debug)]
pub struct SimulationState {
    pub simulation_id: Uuid,
    pub generation: u64,
    pub is_running: bool,
    pub step_interval: f64,
    pub patterns: HashMap<String, Vec<(i32, i32)>>,
}

impl Default for SimulationState {
    fn default() -> Self {
        Self {
            simulation_id: Uuid::new_v4(),
            generation: 0,
            is_running: false,
            step_interval: 1.0,
            patterns: HashMap::new(),
        }
    }
}

impl SimulationState {
    pub fn new() -> Self {
        Self::default()
    }
    
    pub fn start(&mut self) {
        self.is_running = true;
    }
    
    pub fn stop(&mut self) {
        self.is_running = false;
    }
    
    pub fn reset(&mut self) {
        self.generation = 0;
        self.is_running = false;
        self.simulation_id = Uuid::new_v4();
    }
}