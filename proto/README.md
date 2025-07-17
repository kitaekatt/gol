# Protocol Buffer Definitions

This directory contains the Protocol Buffer definitions for the Game of Life gRPC service.

## Files

- `game_of_life.proto` - Main service definition with all messages and RPC methods

## Service Definition

The `GameOfLifeService` defines the following RPC methods:

### Health and Status
- `GetStatus` - Get server health and version information

### Simulation Management
- `CreateSimulation` - Create a new simulation with specified grid dimensions
- `GetSimulation` - Get current state of a simulation
- `UpdateSimulation` - Update simulation state (cells, generation)
- `DeleteSimulation` - Delete a simulation and free resources

### Simulation Control
- `StepSimulation` - Advance simulation by one or more generations
- `LoadPattern` - Load a pattern into the simulation at specified position

### Streaming
- `StreamSimulation` - Stream real-time simulation updates

## Default Ports

- **Bevy**: 50051
- **EnTT**: 50052  
- **Flecs**: 50053

## Code Generation

### Rust (Bevy, Console Client)
```bash
# Add to Cargo.toml
[dependencies]
tonic = "0.10"
prost = "0.12"

[build-dependencies]
tonic-build = "0.10"

# In build.rs
fn main() -> Result<(), Box<dyn std::error::Error>> {
    tonic_build::compile_protos("../proto/game_of_life.proto")?;
    Ok(())
}
```

### C++ (EnTT, Flecs)
```bash
# Generate C++ code
protoc --grpc_out=. --cpp_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` game_of_life.proto
```

## Usage Example

### Client Connection
```rust
use tonic::transport::Channel;
use game_of_life::game_of_life_service_client::GameOfLifeServiceClient;

let channel = Channel::from_static("http://localhost:50051").connect().await?;
let mut client = GameOfLifeServiceClient::new(channel);

let status = client.get_status(StatusRequest {}).await?;
println!("Server: {}", status.into_inner().implementation);
```

### Server Implementation
```rust
use tonic::{transport::Server, Request, Response, Status};
use game_of_life::game_of_life_service_server::{GameOfLifeService, GameOfLifeServiceServer};

#[derive(Debug, Default)]
pub struct GameOfLifeServiceImpl;

#[tonic::async_trait]
impl GameOfLifeService for GameOfLifeServiceImpl {
    async fn get_status(
        &self,
        request: Request<StatusRequest>,
    ) -> Result<Response<StatusResponse>, Status> {
        let response = StatusResponse {
            status: "healthy".to_string(),
            version: "1.0.0".to_string(),
            implementation: "bevy".to_string(),
            uptime_seconds: 3600,
        };
        Ok(Response::new(response))
    }
    
    // ... implement other methods
}
```

## Message Validation

All implementations must validate:
- Grid dimensions (width, height > 0)
- Simulation IDs (valid UUID format)
- Cell coordinates (within grid bounds)
- Pattern data (valid cell positions)
- Step counts (positive integers)

## Error Handling

Use standard gRPC status codes:
- `OK` - Success
- `INVALID_ARGUMENT` - Invalid input parameters
- `NOT_FOUND` - Simulation not found
- `RESOURCE_EXHAUSTED` - Server limits exceeded
- `INTERNAL` - Internal server error