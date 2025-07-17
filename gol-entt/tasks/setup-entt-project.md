# Set Up EnTT Project Infrastructure

## Task Overview
**Priority**: High  
**Assigned Mode**: EnTT  
**Dependencies**: cpp-environment-assessment.md completion
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: None (creates new files)
**Depends On**: cpp-environment-assessment.md
**Modifies Files**: gol-entt/* (entire directory structure)
**Reads Files**: gol-bevy/* (for reference), proto/* (for gRPC definitions)
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 60 minutes
**File Lock Priority**: Medium

## Objective
Establish the EnTT project infrastructure including build system, dependency management, and project structure based on the validated Bevy implementation.

## Context
The Bevy implementation is complete and serves as the functional reference. This task sets up the EnTT project to follow the same API and functional patterns using C++ and EnTT ECS.

## Requirements

### 1. Build System Setup
- Create CMakeLists.txt with proper EnTT integration
- Configure C++17/20 compilation settings
- Set up dependency management (vcpkg, conan, or manual)
- Create build and install targets

### 2. Directory Structure
```
gol-entt/
├── src/
│   ├── components/          # ECS components
│   ├── systems/            # ECS systems
│   ├── resources/          # Global state
│   ├── api/               # gRPC API layer
│   └── main.cpp           # Entry point
├── include/               # Header files
├── tests/                 # Unit tests
├── proto/                 # Protocol buffer definitions
├── benchmarks/            # Performance tests
└── CMakeLists.txt         # Build configuration
```

### 3. Core Dependencies
- **EnTT**: Latest stable version for ECS
- **gRPC**: C++ gRPC libraries for API server
- **Protocol Buffers**: For gRPC communication
- **Testing Framework**: Google Test or Catch2
- **Benchmarking**: Google Benchmark or custom

### 4. Initial Project Files
- main.cpp with basic application structure
- CMakeLists.txt with all dependencies
- Basic component and system header files
- Protocol buffer definitions (copied from Bevy)

## Implementation Steps

### Phase 1: Environment Verification
1. Verify C++ compiler and CMake availability
2. Test dependency installation approach
3. Validate build system functionality

### Phase 2: Project Structure
1. Create directory structure following the layout above
2. Set up CMakeLists.txt with EnTT and gRPC
3. Configure build options and compiler settings
4. Create basic header files for components and systems

### Phase 3: Dependency Integration
1. Install and configure EnTT library
2. Set up gRPC and Protocol Buffers
3. Configure testing framework
4. Validate all dependencies build correctly

### Phase 4: Basic Application
1. Create main.cpp with EnTT registry initialization
2. Add basic gRPC server setup
3. Create placeholder components and systems
4. Verify application builds and runs

## Acceptance Criteria

### Build System
- [ ] CMakeLists.txt builds successfully
- [ ] All dependencies link correctly
- [ ] Debug and release configurations work
- [ ] Tests can be built and run

### Project Structure
- [ ] All required directories created
- [ ] Header files organized properly
- [ ] Protocol buffer definitions available
- [ ] Build artifacts in correct locations

### Dependencies
- [ ] EnTT library integrated and functional
- [ ] gRPC server can be initialized
- [ ] Protocol buffers compile correctly
- [ ] Testing framework operational

### Basic Application
- [ ] main.cpp compiles and runs
- [ ] EnTT registry can be created
- [ ] gRPC server starts without errors
- [ ] Basic components can be created

## Reference Materials
- **Bevy Implementation**: /mnt/c/dev/gol/gol-bevy/ for API and functional reference
- **EnTT Documentation**: Official EnTT repository and examples
- **gRPC C++ Guide**: gRPC C++ tutorial and examples
- **Protocol Buffers**: Use same .proto files as Bevy implementation

## Technical Notes

### EnTT Integration
```cpp
#include <entt/entt.hpp>

// Basic registry setup
entt::registry registry;

// Component registration
struct Position { int x, y; };
struct CellState { bool alive; };

// System example
void update_system(entt::registry& registry) {
    auto view = registry.view<Position, CellState>();
    for (auto entity : view) {
        // Process entity
    }
}
```

### CMake Integration
```cmake
cmake_minimum_required(VERSION 3.15)
project(gol-entt)

set(CMAKE_CXX_STANDARD 17)

find_package(EnTT REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

# Target setup
add_executable(gol-entt src/main.cpp)
target_link_libraries(gol-entt EnTT::EnTT gRPC::grpc++)
```

## Next Steps
Upon completion, the following tasks will be available:
1. implement-entt-components.md - ECS component implementation
2. implement-entt-systems.md - Game logic systems
3. implement-entt-api.md - gRPC API server

## Success Indicators
- Project builds without errors
- Basic EnTT registry functionality works
- gRPC server can be started
- Foundation ready for component implementation
- All dependencies properly integrated