use anyhow::Result;
use crate::client::GameOfLifeClient;
use crate::client::game_of_life::{SimulationResponse, StepResponse, DeleteResponse};

pub struct SimulationCommands {
    client: GameOfLifeClient,
}

impl SimulationCommands {
    pub fn new(client: GameOfLifeClient) -> Self {
        Self { client }
    }
    
    pub async fn create(&mut self, width: i32, height: i32, pattern: Option<String>) -> Result<SimulationResponse> {
        self.client.connect().await?;
        let response = self.client.create_simulation(width, height, pattern).await?;
        println!("Created simulation with ID: {}", response.id);
        Ok(response)
    }
    
    pub async fn get(&mut self, id: String) -> Result<SimulationResponse> {
        self.client.connect().await?;
        let response = self.client.get_simulation(id).await?;
        println!("Simulation ID: {}", response.id);
        println!("Generation: {}", response.generation);
        println!("Live cells: {}", response.live_cells);
        if let Some(grid) = response.grid {
            println!("Grid size: {}x{}", grid.width, grid.height);
        }
        Ok(response)
    }
    
    pub async fn step(&mut self, id: String, steps: i32) -> Result<StepResponse> {
        self.client.connect().await?;
        let response = self.client.step_simulation(id, steps).await?;
        println!("Stepped {} generation(s)", steps);
        println!("Current generation: {}", response.generation);
        println!("Live cells: {}", response.live_cells);
        println!("Changed cells: {}", response.changed_cells);
        Ok(response)
    }
    
    pub async fn delete(&mut self, id: String) -> Result<DeleteResponse> {
        self.client.connect().await?;
        let response = self.client.delete_simulation(id).await?;
        if response.success {
            println!("Simulation deleted successfully");
        } else {
            println!("Failed to delete simulation: {}", response.message);
        }
        Ok(response)
    }
    
    pub async fn status(&mut self) -> Result<String> {
        self.client.connect().await?;
        let status = self.client.get_status().await?;
        let status_text = format!(
            "Server Status: {}\nVersion: {}\nImplementation: {}\nUptime: {} seconds",
            status.status, status.version, status.implementation, status.uptime_seconds
        );
        println!("{}", status_text);
        Ok(status_text)
    }
}