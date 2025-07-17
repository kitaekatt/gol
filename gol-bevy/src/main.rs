use bevy::prelude::*;
use tokio::runtime::Runtime;

mod components;
mod systems;
mod resources;
mod api;
mod grpc;

use components::*;
use systems::*;
use resources::{GridConfig, SimulationState, Simulations};
use api::*;
use grpc::GameOfLifeServiceImpl;

fn main() {
    // Create async runtime for gRPC server
    let rt = Runtime::new().unwrap();
    
    // Start gRPC server in background
    rt.spawn(async {
        start_grpc_server().await.unwrap();
    });
    
    // Start Bevy app
    App::new()
        .add_plugins(MinimalPlugins)
        .init_resource::<GridConfig>()
        .init_resource::<SimulationState>()
        .init_resource::<Simulations>()
        .add_systems(Startup, setup_game)
        .add_systems(Update, (
            neighbor_counting_system,
            cell_lifecycle_system,
            cleanup_system,
        ).chain())
        .run();
}

fn setup_game(
    mut commands: Commands,
    mut grid_config: ResMut<GridConfig>,
    mut simulation_state: ResMut<SimulationState>,
) {
    // Initialize with default grid configuration
    *grid_config = GridConfig::default();
    *simulation_state = SimulationState::new();
    
    info!("Game of Life Bevy server initialized");
}

async fn start_grpc_server() -> Result<(), Box<dyn std::error::Error>> {
    use tonic::transport::Server;
    use grpc::proto::game_of_life_service_server::GameOfLifeServiceServer;
    
    let addr = "[::1]:50051".parse()?;
    let service = GameOfLifeServiceImpl::new();
    
    println!("Starting gRPC Game of Life server on {}", addr);
    
    Server::builder()
        .add_service(GameOfLifeServiceServer::new(service))
        .serve(addr)
        .await?;
    
    Ok(())
}
