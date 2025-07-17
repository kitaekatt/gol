use anyhow::{Result, Context};
use tonic::transport::Channel;
use tonic::{Request, Response, Status};
use std::time::Duration;

pub mod game_of_life {
    tonic::include_proto!("game_of_life");
}

use game_of_life::{
    game_of_life_service_client::GameOfLifeServiceClient,
    StatusRequest, StatusResponse,
    CreateSimulationRequest, SimulationResponse,
    GetSimulationRequest, UpdateSimulationRequest, DeleteSimulationRequest, DeleteResponse,
    StepSimulationRequest, StepResponse,
    LoadPatternRequest, LoadPatternResponse,
    StreamRequest, SimulationUpdate,
    Cell, Position, Pattern, GridInfo,
};

#[derive(Clone)]
pub struct GameOfLifeClient {
    pub backend: String,
    pub host: String,
    pub port: u16,
    pub timeout: Duration,
    client: Option<GameOfLifeServiceClient<Channel>>,
}

impl GameOfLifeClient {
    pub fn new(backend: String, host: String, port: u16) -> Self {
        Self {
            backend,
            host,
            port,
            timeout: Duration::from_secs(5),
            client: None,
        }
    }
    
    pub fn for_backend(backend: &str) -> Self {
        let (host, port) = match backend {
            "bevy" => ("localhost".to_string(), 50051),
            "entt" => ("localhost".to_string(), 50052),
            "flecs" => ("localhost".to_string(), 50053),
            _ => ("localhost".to_string(), 50051),
        };
        
        Self::new(backend.to_string(), host, port)
    }
    
    pub fn with_timeout(mut self, timeout: Duration) -> Self {
        self.timeout = timeout;
        self
    }
    
    pub async fn connect(&mut self) -> Result<()> {
        let endpoint = format!("http://{}:{}", self.host, self.port);
        let channel = Channel::from_shared(endpoint)?
            .timeout(self.timeout)
            .connect()
            .await
            .context("Failed to connect to gRPC server")?;
            
        self.client = Some(GameOfLifeServiceClient::new(channel));
        Ok(())
    }
    
    fn get_client(&mut self) -> Result<&mut GameOfLifeServiceClient<Channel>> {
        self.client.as_mut().ok_or_else(|| {
            anyhow::anyhow!("Client not connected. Call connect() first.")
        })
    }
    
    pub async fn get_status(&mut self) -> Result<StatusResponse> {
        let client = self.get_client()?;
        let request = Request::new(StatusRequest {});
        
        let response = client.get_status(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn create_simulation(&mut self, width: i32, height: i32, initial_pattern: Option<String>) -> Result<SimulationResponse> {
        let client = self.get_client()?;
        let request = Request::new(CreateSimulationRequest {
            width,
            height,
            initial_pattern: initial_pattern.unwrap_or_default(),
        });
        
        let response = client.create_simulation(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn get_simulation(&mut self, id: String) -> Result<SimulationResponse> {
        let client = self.get_client()?;
        let request = Request::new(GetSimulationRequest { id });
        
        let response = client.get_simulation(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn update_simulation(&mut self, id: String, generation: Option<i64>, cells: Option<Vec<Cell>>) -> Result<SimulationResponse> {
        let client = self.get_client()?;
        let request = Request::new(UpdateSimulationRequest {
            id,
            generation: generation.unwrap_or(0),
            cells: cells.unwrap_or_default(),
        });
        
        let response = client.update_simulation(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn delete_simulation(&mut self, id: String) -> Result<DeleteResponse> {
        let client = self.get_client()?;
        let request = Request::new(DeleteSimulationRequest { id });
        
        let response = client.delete_simulation(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn step_simulation(&mut self, id: String, steps: i32) -> Result<StepResponse> {
        let client = self.get_client()?;
        let request = Request::new(StepSimulationRequest { id, steps });
        
        let response = client.step_simulation(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn load_pattern(&mut self, id: String, pattern: Pattern, position: Position) -> Result<LoadPatternResponse> {
        let client = self.get_client()?;
        let request = Request::new(LoadPatternRequest {
            id,
            pattern: Some(pattern),
            position: Some(position),
        });
        
        let response = client.load_pattern(request).await?;
        Ok(response.into_inner())
    }
    
    pub async fn stream_simulation(&mut self, id: String, auto_step: bool, step_interval_ms: i32) -> Result<tonic::Streaming<SimulationUpdate>> {
        let client = self.get_client()?;
        let request = Request::new(StreamRequest {
            id,
            auto_step,
            step_interval_ms,
        });
        
        let response = client.stream_simulation(request).await?;
        Ok(response.into_inner())
    }
}