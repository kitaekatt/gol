# Game of Life - Bevy ECS Implementation

## Implementation Overview

This is a Bevy-based implementation of Conway's Game of Life designed as a gRPC server. The implementation leverages Bevy's Entity-Component-System (ECS) architecture to efficiently simulate the Game of Life using sparse representation where only live cells exist as entities.

**Key Features:**
- Sparse cell representation (only live cells exist)
- Bevy ECS systems for simulation logic
- gRPC server using Tonic for client communication
- High-performance neighbor calculation
- Memory-efficient design (<1KB per 1000 live cells)

## Development Environment Setup

### Prerequisites
- **Rust**: Version 1.70+ (use `rustup` to install)
- **Cargo**: Included with Rust
- **Protocol Buffers**: For gRPC code generation
  ```bash
  # Ubuntu/Debian
  sudo apt install protobuf-compiler
  
  # macOS
  brew install protobuf
  
  # Windows
  # Download from https://github.com/protocolbuffers/protobuf/releases
  ```

### Installation
1. **Clone the repository** (if not already done)
2. **Navigate to the Bevy implementation**:
   ```bash
   cd gol-bevy
   ```
3. **Install dependencies**:
   ```bash
   cargo build
   ```

### Dependencies
- **Bevy**: ECS framework for game logic
- **Tonic**: gRPC server implementation
- **Prost**: Protocol Buffers for Rust
- **Tokio**: Async runtime for server operations
- **UUID**: Unique simulation identifiers
- **Serde**: JSON serialization for patterns

## Architecture Summary

### ECS Components
- **Position**: Cell coordinates (x, y)
- **CellState**: Cell lifecycle state (Alive, Dead, New)
- **NeighborCount**: Number of living neighbors (0-8)

### ECS Systems
- **NeighborSystem**: Calculates neighbor counts for all cells
- **LifecycleSystem**: Applies Game of Life rules to determine cell fate
- **ApiSystem**: Handles gRPC client requests and responses
- **CleanupSystem**: Removes dead cells to maintain sparse representation

### ECS Resources
- **GridConfig**: Grid dimensions and configuration
- **GenerationCounter**: Current simulation generation
- **SimulationState**: Global simulation state and metadata

### System Execution Order
1. **NeighborSystem** - Calculate neighbors for all cells
2. **LifecycleSystem** - Apply Game of Life rules
3. **CleanupSystem** - Remove dead cells
4. **ApiSystem** - Handle client requests

## Build Instructions

### Development Build
```bash
cargo build
```

### Release Build
```bash
cargo build --release
```

### Running the Server
```bash
# Development mode (with debug logging)
cargo run

# Release mode (optimized)
cargo run --release

# Custom port (default: 50051)
cargo run -- --port 50052
```

### Server Configuration
The server can be configured via environment variables:
- `GRPC_PORT`: Server port (default: 50051)
- `LOG_LEVEL`: Log level (debug, info, warn, error)
- `MAX_SIMULATIONS`: Maximum concurrent simulations (default: 10)

## Testing Guidelines

### Running Tests
```bash
# Run all tests
cargo test

# Run unit tests only
cargo test --lib

# Run integration tests
cargo test --test '*'

# Run with output
cargo test -- --nocapture

# Run specific test
cargo test test_glider_pattern
```

### Test Categories
- **Unit Tests**: Individual component and system behavior
- **Integration Tests**: gRPC service functionality
- **Pattern Tests**: Validation of Game of Life patterns
- **Performance Tests**: Memory usage and computation benchmarks

### Test Patterns
The implementation validates against standard Game of Life patterns:
- **Still Life**: Block, Beehive, Loaf
- **Oscillators**: Blinker, Toad, Beacon
- **Spaceships**: Glider, Lightweight Spaceship
- **Complex**: Glider Gun, Acorn, Diehard

## API Documentation

### gRPC Service Interface
The server implements the `GameOfLifeService` as defined in `../proto/game_of_life.proto`.

#### Key Methods
- `GetStatus()`: Server health and implementation info
- `CreateSimulation()`: Create new simulation with initial pattern
- `GetSimulation()`: Retrieve current simulation state
- `StepSimulation()`: Advance simulation by N generations
- `LoadPattern()`: Add pattern to existing simulation
- `StreamSimulation()`: Real-time simulation updates

#### Example Usage
```rust
// Create simulation
let response = client.create_simulation(CreateSimulationRequest {
    width: 100,
    height: 100,
    initial_pattern: "glider".to_string(),
}).await?;

// Step simulation
let step_response = client.step_simulation(StepSimulationRequest {
    id: response.id,
    steps: 10,
}).await?;
```

### Pattern Format
Patterns are loaded from JSON files in the `../patterns/` directory:
```json
{
  "name": "Glider",
  "description": "A simple spaceship",
  "author": "John Conway",
  "cells": [
    {"x": 1, "y": 0},
    {"x": 2, "y": 1},
    {"x": 0, "y": 2}
  ]
}
```

## Performance Targets

### Memory Efficiency
- **Target**: <1KB per 1000 live cells
- **Sparse Storage**: Only live cells exist as entities
- **Efficient Cleanup**: Dead cells removed immediately
- **Memory Tracking**: Built-in memory usage monitoring

### Computational Performance
- **Complexity**: O(live_cells) not O(grid_size²)
- **Scalability**: Support up to 10,000x10,000 grids
- **Response Time**: gRPC calls complete within 100ms
- **Throughput**: Handle 10+ concurrent simulations

### Benchmarking
```bash
# Run performance benchmarks
cargo bench

# Memory usage analysis
cargo run --example memory_test

# Throughput testing
cargo run --example concurrent_simulations
```

## Troubleshooting

### Common Issues

#### Build Errors
**Issue**: `protobuf compiler not found`
**Solution**: Install protobuf compiler:
```bash
sudo apt install protobuf-compiler  # Ubuntu/Debian
brew install protobuf               # macOS
```

**Issue**: `edition 2024 not supported`
**Solution**: Update Rust to latest version:
```bash
rustup update stable
```

#### Runtime Errors
**Issue**: `gRPC server failed to start`
**Solution**: Check if port is available:
```bash
netstat -tlnp | grep 50051
```

**Issue**: `Pattern file not found`
**Solution**: Verify pattern files exist in `../patterns/` directory:
```bash
ls -la ../patterns/
```

#### Performance Issues
**Issue**: High memory usage
**Solution**: Check for memory leaks in cleanup system:
```bash
cargo run --example memory_profile
```

**Issue**: Slow simulation steps
**Solution**: Enable release optimizations:
```bash
cargo run --release
```

### Debugging
Enable debug logging:
```bash
RUST_LOG=debug cargo run
```

View system execution order:
```bash
RUST_LOG=bevy_ecs=debug cargo run
```

### Getting Help
- **Documentation**: Run `cargo doc --open` for API docs
- **Examples**: Check `examples/` directory for usage patterns
- **Issues**: Report bugs in project issue tracker
- **Performance**: Use `cargo flamegraph` for profiling

## Development Notes

### Bevy-Specific Patterns
- **Systems**: Use Bevy's system parameter injection
- **Queries**: Leverage Bevy's powerful query system
- **Resources**: Store global state in Bevy resources
- **Events**: Use Bevy events for decoupled communication

### Code Organization
```
src/
├── main.rs              # Server entry point
├── lib.rs               # Library interface
├── components/          # ECS components
│   ├── mod.rs
│   ├── position.rs
│   ├── cell_state.rs
│   └── neighbor.rs
├── systems/             # ECS systems
│   ├── mod.rs
│   ├── neighbor.rs
│   ├── lifecycle.rs
│   ├── cleanup.rs
│   └── api.rs
├── resources/           # ECS resources
│   ├── mod.rs
│   ├── grid_config.rs
│   └── simulation.rs
└── grpc/               # gRPC service implementation
    ├── mod.rs
    └── service.rs
```

### Best Practices
- **Entity Lifecycle**: Create entities for live cells only
- **System Ordering**: Ensure proper execution order
- **Resource Management**: Use Bevy's resource system
- **Error Handling**: Implement comprehensive error handling
- **Performance**: Profile regularly and optimize hot paths

This implementation provides a solid foundation for the Game of Life simulation while maintaining the flexibility and performance characteristics of the Bevy ECS framework.