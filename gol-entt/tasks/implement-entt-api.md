# Implement EnTT gRPC API Server

## Task Overview
**Priority**: High  
**Assigned Mode**: EnTT  
**Dependencies**: implement-entt-systems.md completion
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: implement-entt-components.md, implement-entt-systems.md
**Depends On**: implement-entt-systems.md
**Modifies Files**: gol-entt/src/api/*, gol-entt/proto/*
**Reads Files**: proto/* (shared gRPC definitions), gol-bevy/src/api/*
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 60 minutes
**File Lock Priority**: Medium

## Objective
Implement the gRPC API server for the EnTT Game of Life implementation, ensuring API compatibility with the Bevy implementation for seamless client integration.

## Context
The Bevy implementation provides a complete gRPC API server running on port 50051. This task implements an identical API using the EnTT ECS implementation, allowing the console client to work with both servers interchangeably.

## Requirements

### 1. gRPC Service Implementation
- Implement all API endpoints from the Bevy reference
- Use same Protocol Buffer definitions
- Ensure API compatibility and identical behavior
- Handle all request/response patterns

### 2. Core API Endpoints
Based on the Bevy reference implementation:

#### Simulation Control
- `StartSimulation(StartSimulationRequest) -> StartSimulationResponse`
- `StopSimulation(StopSimulationRequest) -> StopSimulationResponse`
- `ResetSimulation(ResetSimulationRequest) -> ResetSimulationResponse`
- `StepSimulation(StepSimulationRequest) -> StepSimulationResponse`

#### Pattern Management
- `LoadPattern(LoadPatternRequest) -> LoadPatternResponse`
- `SavePattern(SavePatternRequest) -> SavePatternResponse`
- `ListPatterns(ListPatternsRequest) -> ListPatternsResponse`

#### State Access
- `GetGrid(GetGridRequest) -> GetGridResponse`
- `GetStatus(GetStatusRequest) -> GetStatusResponse`
- `GetMetrics(GetMetricsRequest) -> GetMetricsResponse`

### 3. Server Infrastructure
- gRPC server setup and configuration
- Request routing and handling
- Error handling and status codes
- Server lifecycle management

## Implementation Steps

### Phase 1: Protocol Buffer Setup
1. Copy .proto files from Bevy implementation
2. Generate C++ gRPC code
3. Set up CMake integration for proto compilation
4. Validate proto compilation

### Phase 2: gRPC Server Framework
1. Create gRPC server class and initialization
2. Implement service interface structure
3. Set up request routing
4. Add server lifecycle management

### Phase 3: API Handler Implementation
1. Implement simulation control endpoints
2. Add pattern management functionality
3. Create state access endpoints
4. Handle request/response conversion

### Phase 4: Integration and Testing
1. Integrate API with EnTT ECS systems
2. Test API endpoints with real clients
3. Validate compatibility with Bevy API
4. Performance testing and optimization

## Acceptance Criteria

### API Implementation
- [ ] All gRPC endpoints implemented
- [ ] Protocol buffer integration working
- [ ] Request/response handling correct
- [ ] Error handling comprehensive

### Compatibility
- [ ] API identical to Bevy implementation
- [ ] Same request/response formats
- [ ] Compatible with existing console client
- [ ] Identical behavior for all endpoints

### Integration
- [ ] EnTT ECS systems integrated with API
- [ ] State conversion working correctly
- [ ] Performance meets requirements
- [ ] Server runs on port 50051 (configurable)

### Testing
- [ ] API endpoints tested with real clients
- [ ] Cross-implementation compatibility verified
- [ ] Error conditions handled properly
- [ ] Performance benchmarks pass

## Reference Implementation
Use the Bevy implementation as functional reference:
- **API Server**: /mnt/c/dev/gol/gol-bevy/src/api/
- **gRPC Service**: /mnt/c/dev/gol/gol-bevy/src/grpc/
- **Protocol Buffers**: /mnt/c/dev/gol/gol-bevy/proto/

## Technical Specifications

### gRPC Service Class
```cpp
// In api/game_of_life_service.hpp
#pragma once
#include <grpc++/grpc++.h>
#include <entt/entt.hpp>
#include "game_of_life.grpc.pb.h"

class GameOfLifeService final : public gameoflife::GameOfLifeService::Service {
public:
    explicit GameOfLifeService(entt::registry& registry);
    
    // Simulation control
    grpc::Status StartSimulation(grpc::ServerContext* context,
                               const gameoflife::StartSimulationRequest* request,
                               gameoflife::StartSimulationResponse* response) override;
    
    grpc::Status StopSimulation(grpc::ServerContext* context,
                              const gameoflife::StopSimulationRequest* request,
                              gameoflife::StopSimulationResponse* response) override;
    
    grpc::Status ResetSimulation(grpc::ServerContext* context,
                               const gameoflife::ResetSimulationRequest* request,
                               gameoflife::ResetSimulationResponse* response) override;
    
    grpc::Status StepSimulation(grpc::ServerContext* context,
                              const gameoflife::StepSimulationRequest* request,
                              gameoflife::StepSimulationResponse* response) override;
    
    // Pattern management
    grpc::Status LoadPattern(grpc::ServerContext* context,
                           const gameoflife::LoadPatternRequest* request,
                           gameoflife::LoadPatternResponse* response) override;
    
    // State access
    grpc::Status GetGrid(grpc::ServerContext* context,
                        const gameoflife::GetGridRequest* request,
                        gameoflife::GetGridResponse* response) override;
    
    grpc::Status GetStatus(grpc::ServerContext* context,
                         const gameoflife::GetStatusRequest* request,
                         gameoflife::GetStatusResponse* response) override;
    
private:
    entt::registry& registry_;
    // Add simulation state management
};
```

### gRPC Server Setup
```cpp
// In api/server.hpp
#pragma once
#include <grpc++/grpc++.h>
#include <memory>
#include <string>

class GrpcServer {
public:
    explicit GrpcServer(const std::string& address = "0.0.0.0:50051");
    ~GrpcServer();
    
    void start();
    void stop();
    void wait();
    
private:
    std::unique_ptr<grpc::Server> server_;
    std::string address_;
};
```

### State Conversion Utilities
```cpp
// In api/converters.hpp
#pragma once
#include <entt/entt.hpp>
#include "game_of_life.pb.h"
#include "../components/position.hpp"
#include "../components/cell_state.hpp"

class StateConverter {
public:
    static gameoflife::Grid registry_to_grid(entt::registry& registry);
    static void grid_to_registry(const gameoflife::Grid& grid, entt::registry& registry);
    static gameoflife::Cell cell_to_proto(const Position& pos, const CellState& state);
    static void proto_to_cell(const gameoflife::Cell& cell, entt::registry& registry);
};
```

### CMake Integration
```cmake
# Add to CMakeLists.txt
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Protocol buffer compilation
set(PROTO_FILES
    proto/game_of_life.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILES})

# Add generated files to target
target_sources(gol-entt PRIVATE
    ${PROTO_SRCS}
    ${GRPC_SRCS}
    src/api/game_of_life_service.cpp
    src/api/server.cpp
    src/api/converters.cpp
)

target_link_libraries(gol-entt
    gRPC::grpc++
    protobuf::libprotobuf
)
```

## API Implementation Details

### Simulation Control
- Start/stop simulation with proper state management
- Reset simulation to clear state
- Step simulation for single-step execution
- Handle concurrent access and threading

### Pattern Management
- Load patterns from standard formats
- Save current state as patterns
- List available patterns
- Pattern validation and error handling

### State Access
- Convert EnTT registry to gRPC grid format
- Provide simulation status and metrics
- Handle large grid serialization efficiently
- Optimize for frequent state queries

## Performance Considerations

### gRPC Optimization
- Use efficient serialization
- Minimize memory allocations
- Optimize for large grid responses
- Handle concurrent requests

### State Conversion
- Efficient registry to proto conversion
- Minimize data copying
- Optimize for sparse grids
- Cache conversion results when possible

## Next Steps
Upon completion, the following tasks will be available:
1. entt-testing-framework.md - Comprehensive testing
2. entt-performance-optimization.md - Performance tuning
3. entt-cross-validation.md - Validation against Bevy

## Success Indicators
- gRPC server starts and accepts connections
- All API endpoints respond correctly
- Compatible with existing console client
- Performance meets requirements
- API behavior identical to Bevy implementation
- Ready for integration testing