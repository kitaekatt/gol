use anyhow::Result;
use std::time::Duration;
use tokio::time;
use crate::client::GameOfLifeClient;
use crate::client::game_of_life::SimulationUpdate;

pub struct ControlCommands {
    client: GameOfLifeClient,
    current_simulation: Option<String>,
    auto_step_interval: Duration,
}

impl ControlCommands {
    pub fn new(client: GameOfLifeClient) -> Self {
        Self {
            client,
            current_simulation: None,
            auto_step_interval: Duration::from_millis(1000),
        }
    }
    
    pub fn set_current_simulation(&mut self, id: String) {
        self.current_simulation = Some(id);
    }
    
    pub fn get_current_simulation(&self) -> Option<&String> {
        self.current_simulation.as_ref()
    }
    
    pub fn set_speed(&mut self, interval_ms: u64) {
        self.auto_step_interval = Duration::from_millis(interval_ms);
        println!("Auto-step interval set to {}ms", interval_ms);
    }
    
    pub async fn play(&mut self, simulation_id: Option<String>) -> Result<()> {
        let id = simulation_id.or_else(|| self.current_simulation.clone())
            .ok_or_else(|| anyhow::anyhow!("No simulation ID provided and no current simulation set"))?;
        
        println!("Starting auto-play for simulation: {}", id);
        println!("Press Ctrl+C to stop");
        
        self.client.connect().await?;
        
        let mut generation = 0;
        loop {
            match self.client.step_simulation(id.clone(), 1).await {
                Ok(response) => {
                    generation = response.generation;
                    println!("Generation: {}, Live cells: {}", generation, response.live_cells);
                    
                    if response.live_cells == 0 {
                        println!("Simulation ended - no live cells remaining");
                        break;
                    }
                }
                Err(e) => {
                    println!("Error stepping simulation: {}", e);
                    break;
                }
            }
            
            time::sleep(self.auto_step_interval).await;
        }
        
        Ok(())
    }
    
    pub async fn stream(&mut self, simulation_id: Option<String>) -> Result<()> {
        let id = simulation_id.or_else(|| self.current_simulation.clone())
            .ok_or_else(|| anyhow::anyhow!("No simulation ID provided and no current simulation set"))?;
        
        println!("Starting streaming for simulation: {}", id);
        println!("Press Ctrl+C to stop");
        
        self.client.connect().await?;
        
        let mut stream = self.client.stream_simulation(
            id.clone(),
            true,
            self.auto_step_interval.as_millis() as i32
        ).await?;
        
        while let Some(update) = stream.message().await? {
            println!("Generation: {}, Live cells: {}, Changed cells: {}", 
                     update.generation, update.live_cells, update.changed_cells.len());
            
            if update.simulation_ended {
                println!("Simulation ended - reached stable state");
                break;
            }
        }
        
        Ok(())
    }
    
    pub async fn pause(&mut self) -> Result<()> {
        println!("Simulation paused (streaming stopped)");
        Ok(())
    }
    
    pub fn switch_backend(&mut self, backend: &str) -> Result<()> {
        let new_client = GameOfLifeClient::for_backend(backend);
        self.client = new_client;
        println!("Switched to {} backend", backend);
        Ok(())
    }
    
    pub fn get_backend_info(&self) -> String {
        format!("Current backend: {}", self.client.backend)
    }
    
    pub async fn reset_simulation(&mut self, simulation_id: String) -> Result<()> {
        self.client.connect().await?;
        
        let sim_info = self.client.get_simulation(simulation_id.clone()).await?;
        let grid = sim_info.grid.ok_or_else(|| anyhow::anyhow!("No grid information available"))?;
        
        let _delete_response = self.client.delete_simulation(simulation_id).await?;
        
        let new_sim = self.client.create_simulation(grid.width, grid.height, None).await?;
        
        self.current_simulation = Some(new_sim.id.clone());
        println!("Simulation reset. New ID: {}", new_sim.id);
        
        Ok(())
    }
    
    pub fn show_controls(&self) {
        println!("Available controls:");
        println!("  play [simulation_id]  - Auto-step simulation");
        println!("  stream [simulation_id] - Stream real-time updates");
        println!("  pause                 - Pause auto-stepping");
        println!("  speed <ms>            - Set auto-step interval");
        println!("  backend <name>        - Switch backend (bevy|entt|flecs)");
        println!("  reset <simulation_id> - Reset simulation to empty state");
        println!("  current <simulation_id> - Set current simulation");
        println!("  info                  - Show current backend info");
    }
}