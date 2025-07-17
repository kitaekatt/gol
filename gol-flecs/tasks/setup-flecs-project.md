# Set Up Flecs Project Infrastructure

## Task Overview
**Priority**: High  
**Assigned Mode**: Flecs  
**Dependencies**: EnTT implementation completion for C++ patterns
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: None (creates new files)
**Depends On**: EnTT implementation completion
**Modifies Files**: gol-flecs/* (entire directory structure)
**Reads Files**: gol-bevy/* (for reference), gol-entt/* (for C++ patterns), proto/*
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 60 minutes
**File Lock Priority**: Medium

## Objective
Establish the Flecs project infrastructure including build system, dependency management, and project structure based on the validated Bevy implementation and EnTT C++ patterns.

## Context
The Bevy implementation is complete and the EnTT implementation provides C++ patterns. This task sets up the Flecs project to follow the same API and functional patterns using C++ and Flecs ECS.

## Requirements

### 1. Build System Setup
- Create CMakeLists.txt with proper Flecs integration
- Configure C++17/20 compilation settings
- Set up dependency management for Flecs
- Create build and install targets

### 2. Directory Structure
```
gol-flecs/
├── src/
│   ├── components/          # ECS components
│   ├── systems/            # ECS systems
│   ├── modules/            # Flecs modules
│   ├── api/               # gRPC API layer
│   └── main.cpp           # Entry point
├── include/               # Header files
├── tests/                 # Unit tests
├── proto/                 # Protocol buffer definitions
├── benchmarks/            # Performance tests
└── CMakeLists.txt         # Build configuration
```

### 3. Core Dependencies
- **Flecs**: Latest stable version for ECS
- **gRPC**: C++ gRPC libraries for API server
- **Protocol Buffers**: For gRPC communication
- **Testing Framework**: Google Test or Catch2
- **Benchmarking**: Google Benchmark or custom

### 4. Initial Project Files
- main.cpp with basic Flecs world setup
- CMakeLists.txt with all dependencies
- Basic component and system definitions
- Protocol buffer definitions (copied from Bevy)

## Implementation Steps

### Phase 1: Environment Verification
1. Verify C++ compiler and CMake availability
2. Test Flecs library installation
3. Validate build system functionality
4. Test gRPC integration

### Phase 2: Project Structure
1. Create directory structure following the layout above
2. Set up CMakeLists.txt with Flecs and gRPC
3. Configure build options and compiler settings
4. Create basic header files for components and systems

### Phase 3: Dependency Integration
1. Install and configure Flecs library
2. Set up gRPC and Protocol Buffers
3. Configure testing framework
4. Validate all dependencies build correctly

### Phase 4: Basic Application
1. Create main.cpp with Flecs world initialization
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
- [ ] Flecs library integrated and functional
- [ ] gRPC server can be initialized
- [ ] Protocol buffers compile correctly
- [ ] Testing framework operational

### Basic Application
- [ ] main.cpp compiles and runs
- [ ] Flecs world can be created
- [ ] gRPC server starts without errors
- [ ] Basic components can be registered

## Reference Materials
- **Bevy Implementation**: /mnt/c/dev/gol/gol-bevy/ for API and functional reference
- **EnTT Implementation**: /mnt/c/dev/gol/gol-entt/ for C++ patterns
- **Flecs Documentation**: Official Flecs repository and examples
- **gRPC C++ Guide**: gRPC C++ tutorial and examples

## Technical Notes

### Flecs Integration
```cpp
#include <flecs.h>

// Basic world setup
flecs::world world;

// Component registration
struct Position {
    int x, y;
};

struct CellState {
    bool alive;
};

// Component registration
world.component<Position>();
world.component<CellState>();

// System example
world.system<Position, CellState>()
    .each([](Position& pos, CellState& state) {
        // Process entity
    });
```

### CMake Integration
```cmake
cmake_minimum_required(VERSION 3.15)
project(gol-flecs)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FLECS REQUIRED flecs)

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

# Target setup
add_executable(gol-flecs src/main.cpp)
target_link_libraries(gol-flecs ${FLECS_LIBRARIES} gRPC::grpc++)
target_include_directories(gol-flecs PRIVATE ${FLECS_INCLUDE_DIRS})
```

### Flecs Module Structure
```cpp
// In modules/game_of_life_module.hpp
#pragma once
#include <flecs.h>

class GameOfLifeModule {
public:
    static void import(flecs::world& world);
    
private:
    static void register_components(flecs::world& world);
    static void register_systems(flecs::world& world);
};
```

## Next Steps
Upon completion, the following tasks will be available:
1. implement-flecs-components.md - ECS component implementation
2. implement-flecs-systems.md - Game logic systems
3. implement-flecs-api.md - gRPC API server

## Success Indicators
- Project builds without errors
- Basic Flecs world functionality works
- gRPC server can be started
- Foundation ready for component implementation
- All dependencies properly integrated
- Flecs-specific patterns established