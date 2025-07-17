# Game of Life Multi-Implementation Architecture

## Overview

This document defines the unified architecture for three functionally equivalent Game of Life implementations, emphasizing server-only design with a shared console client for testing and validation.

## Core Architecture Principles

### 1. Client-Server Separation
- **Server Implementations**: Each ECS implementation (Bevy, EnTT, Flecs) operates as a standalone server
- **Unified Client**: Single console client communicates with any server implementation
- **gRPC Protocol**: All servers expose identical gRPC service definitions
- **Interoperability**: Client can switch between backends seamlessly

### 2. ECS Architecture Foundation
All implementations follow Entity-Component-System patterns with:
- **Entities**: Represent individual cells (only live cells exist)
- **Components**: Store cell data (position, state, neighbor count)
- **Systems**: Execute game logic (neighbor calculation, lifecycle management)

### 3. Sparse Representation
- Only live cells exist as entities
- Dramatically reduces memory usage for typical patterns
- Enables efficient processing of large, sparse grids
- Target: <1KB memory per 1000 live cells

## Implementation Architecture

### Server Structure (All Implementations)

```
Implementation Root/
├── src/
│   ├── components/          # ECS components
│   │   ├── position.rs/hpp  # Cell coordinates
│   │   ├── cell_state.rs/hpp # Alive/dead state
│   │   └── neighbor.rs/hpp  # Neighbor count
│   ├── systems/            # ECS systems
│   │   ├── neighbor_system.rs/hpp # Calculate neighbors
│   │   ├── lifecycle_system.rs/hpp # Apply GoL rules
│   │   └── api_system.rs/hpp # Handle client requests
│   ├── resources/          # Global state
│   │   ├── grid_config.rs/hpp # Grid parameters
│   │   └── generation.rs/hpp # Time tracking
│   ├── api/               # REST API layer
│   │   ├── handlers.rs/hpp # HTTP request handlers
│   │   └── models.rs/hpp   # JSON data structures
│   └── server.rs/cpp      # Main server entry point
├── tasks/                 # Task definitions for agents
├── patterns/              # JSON pattern files
├── tests/                 # Unit and integration tests
└── benchmarks/            # Performance benchmarks
```

### Console Client Structure

```
gol-console-client/
├── src/
│   ├── client.rs          # HTTP client implementation
│   ├── ui/               # Terminal interface
│   │   ├── display.rs    # Grid visualization
│   │   ├── input.rs      # User input handling
│   │   └── menu.rs       # Command interface
│   ├── commands/         # Client commands
│   │   ├── simulate.rs   # Run simulation
│   │   ├── load.rs       # Load patterns
│   │   └── benchmark.rs  # Performance testing
│   └── main.rs           # Entry point
├── tasks/                # Task definitions
└── tests/               # Client tests
```

## gRPC Service Specification

### Technology Choice: gRPC

**Why gRPC was chosen:**
- **Performance**: Binary Protocol Buffers are more efficient than JSON
- **Type Safety**: Strong typing across all languages (Rust, C++)
- **Streaming**: Supports real-time simulation updates
- **Code Generation**: Automatic client/server code generation
- **Cross-Language**: Consistent API across Bevy (Rust), EnTT (C++), Flecs (C++)
- **Versioning**: Built-in API versioning and backward compatibility

### Core Service Methods
All server implementations must implement these gRPC methods:

```protobuf
service GameOfLifeService {
  rpc GetStatus(StatusRequest) returns (StatusResponse);
  rpc CreateSimulation(CreateSimulationRequest) returns (SimulationResponse);
  rpc GetSimulation(GetSimulationRequest) returns (SimulationResponse);
  rpc UpdateSimulation(UpdateSimulationRequest) returns (SimulationResponse);
  rpc DeleteSimulation(DeleteSimulationRequest) returns (DeleteResponse);
  rpc StepSimulation(StepSimulationRequest) returns (StepResponse);
  rpc LoadPattern(LoadPatternRequest) returns (LoadPatternResponse);
  rpc StreamSimulation(StreamRequest) returns (stream SimulationUpdate);
}
```

### Protocol Buffer Messages

#### Cell Message
```protobuf
message Cell {
  int32 x = 1;
  int32 y = 2;
  bool alive = 3;
  int32 neighbors = 4;
}
```

#### Simulation State Message
```protobuf
message SimulationResponse {
  string id = 1;
  int64 generation = 2;
  int64 live_cells = 3;
  GridInfo grid = 4;
  repeated Cell cells = 5;
}

message GridInfo {
  int32 width = 1;
  int32 height = 2;
}
```

#### Pattern Message
```protobuf
message Pattern {
  string name = 1;
  string description = 2;
  string author = 3;
  repeated Position cells = 4;
}

message Position {
  int32 x = 1;
  int32 y = 2;
}
```

## Technology-Specific Considerations

### Bevy Implementation (Rust)
- **gRPC Server**: Use `tonic` for gRPC service implementation
- **ECS Integration**: Bevy systems handle simulation logic
- **Resource Management**: Bevy resources for global state
- **Scheduling**: Bevy's scheduler for system execution
- **Protocol Buffers**: `prost` for message serialization

### EnTT Implementation (C++)
- **gRPC Server**: Use `grpc++` for gRPC service implementation
- **ECS Integration**: EnTT registry and systems
- **Memory Management**: RAII and smart pointers
- **Threading**: Consider thread-safe operations
- **Protocol Buffers**: `protobuf` for message serialization

### Flecs Implementation (C++)
- **gRPC Server**: Use `grpc++` for gRPC service implementation
- **ECS Integration**: Flecs world and systems
- **Query System**: Leverage Flecs advanced querying
- **Threading**: Flecs threading support
- **Protocol Buffers**: `protobuf` for message serialization

## Performance Requirements

### Memory Efficiency
- **Target**: <1KB per 1000 live cells
- **Sparse Storage**: Only track live cells
- **Efficient Indexing**: Use spatial hashing for neighbor lookups
- **Garbage Collection**: Timely cleanup of dead cells

### Computational Performance
- **Scalability**: Support grids up to 10,000x10,000
- **Complexity**: O(live_cells) instead of O(grid_size²)
- **Parallel Processing**: Leverage multi-core systems
- **Cache Efficiency**: Memory-friendly data layouts

### Network Performance
- **Response Time**: <100ms for gRPC calls
- **Throughput**: Support multiple concurrent clients
- **Compression**: Efficient Protocol Buffer serialization
- **Streaming**: Real-time simulation updates via gRPC streaming

## Testing Architecture

### Unit Testing
- **Component Tests**: Individual ECS components
- **System Tests**: ECS system behavior
- **API Tests**: HTTP endpoint validation
- **Performance Tests**: Benchmark critical paths

### Integration Testing
- **End-to-End**: gRPC client-server communication
- **Pattern Loading**: Verify pattern correctness
- **Simulation Accuracy**: Validate Game of Life rules
- **Cross-Implementation**: Ensure identical behavior
- **gRPC Testing**: Use gRPC testing frameworks for service validation

### Validation Framework
- **Golden Tests**: Known pattern outcomes
- **Regression Tests**: Prevent behavior changes
- **Performance Benchmarks**: Track optimization
- **Interoperability Tests**: Cross-backend validation

## Development Workflow

### Task-Based Development
1. **Task Definition**: Create .md files in `tasks/` directories
2. **Agent Assignment**: Claude Code agents work on individual tasks
3. **Implementation**: Follow specifications in REQUIREMENTS.md
4. **Testing**: Validate against acceptance criteria
5. **Completion**: Delete task file and report to orchestrator

### Quality Gates
- **Functional Equivalence**: All implementations produce identical results
- **Performance Targets**: Meet memory and computational requirements
- **gRPC Service Compliance**: Conform to unified Protocol Buffer definitions
- **Test Coverage**: Comprehensive unit and integration tests

## Deployment and Operations

### Server Deployment
- **Containerization**: Docker containers for each implementation
- **Process Management**: Systemd or similar for service management
- **Monitoring**: Health checks and performance metrics
- **Logging**: Structured logging for debugging

### Client Distribution
- **Single Binary**: Portable executable for all platforms
- **Configuration**: Support for different server endpoints
- **Documentation**: Built-in help and examples
- **Extensibility**: Plugin system for additional backends

## Future Considerations

### Extensibility
- **Plugin Architecture**: Support for additional ECS implementations
- **Custom Patterns**: User-defined pattern formats
- **Visualization**: GUI client implementations
- **Analytics**: Performance and usage metrics

### Optimization
- **SIMD Instructions**: Vector operations for performance
- **GPU Acceleration**: Consider GPU-based implementations
- **Distributed Computing**: Multi-node simulations
- **Caching**: Intelligent state caching strategies