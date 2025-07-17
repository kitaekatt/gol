# Implement EnTT ECS Components

## Task Overview
**Priority**: High  
**Assigned Mode**: EnTT  
**Dependencies**: setup-entt-project.md completion
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: implement-entt-systems.md, implement-entt-api.md
**Depends On**: setup-entt-project.md
**Modifies Files**: gol-entt/src/components/*, gol-entt/include/components/*
**Reads Files**: gol-bevy/src/components/* (for reference)
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 45 minutes
**File Lock Priority**: Medium

## Objective
Implement the core ECS components for the Game of Life using EnTT patterns, following the functional specification established by the Bevy implementation.

## Context
The Bevy implementation uses Position and CellState components with sparse representation. This task implements equivalent components using EnTT's component system.

## Requirements

### 1. Core Components
Based on the Bevy reference implementation:

#### Position Component
```cpp
struct Position {
    int32_t x;
    int32_t y;
    
    // Neighbor calculation methods
    std::vector<Position> get_neighbors() const;
    bool operator==(const Position& other) const;
    bool operator<(const Position& other) const; // For std::map usage
};
```

#### CellState Component
```cpp
struct CellState {
    bool alive;
    uint32_t generation;
    uint8_t neighbor_count;
    
    // State transition methods
    bool should_survive() const;
    bool should_be_born() const;
    void update_generation();
};
```

### 2. Component Registration
- Register components with EnTT registry
- Implement proper component lifecycle
- Create component factory functions
- Add component serialization support

### 3. Component Utilities
- Neighbor calculation functions
- Component creation helpers
- State transition utilities
- Memory management patterns

## Implementation Steps

### Phase 1: Component Definitions
1. Create header files for Position and CellState
2. Implement constructors and basic methods
3. Add neighbor calculation logic
4. Create equality and comparison operators

### Phase 2: EnTT Integration
1. Register components with EnTT registry
2. Test component creation and destruction
3. Implement component access patterns
4. Validate component storage efficiency

### Phase 3: Game Logic Integration
1. Implement Conway's Game of Life rules
2. Add state transition methods
3. Create neighbor counting logic
4. Test component interactions

### Phase 4: Testing and Validation
1. Create unit tests for all components
2. Test component lifecycle management
3. Validate game logic correctness
4. Performance testing for component operations

## Acceptance Criteria

### Component Implementation
- [ ] Position component with neighbor calculation
- [ ] CellState component with game logic
- [ ] Proper EnTT registration and lifecycle
- [ ] Component factory functions

### Game Logic
- [ ] Conway's Game of Life rules implemented
- [ ] Neighbor counting algorithm
- [ ] State transition logic
- [ ] Sparse representation support

### Testing
- [ ] Unit tests for all components
- [ ] Game logic validation tests
- [ ] Performance benchmarks
- [ ] Memory usage validation

### Integration
- [ ] Components work with EnTT registry
- [ ] Proper component access patterns
- [ ] Efficient component storage
- [ ] Ready for system integration

## Reference Implementation
Use the Bevy implementation as functional reference:
- **Components**: /mnt/c/dev/gol/gol-bevy/src/components/
- **Game Logic**: Conway's Game of Life rules
- **API**: Component data structures for gRPC

## Technical Specifications

### Position Component Details
```cpp
// In components/position.hpp
#pragma once
#include <vector>
#include <cstdint>

struct Position {
    int32_t x;
    int32_t y;
    
    Position(int32_t x = 0, int32_t y = 0) : x(x), y(y) {}
    
    // Get all 8 neighbors
    std::vector<Position> get_neighbors() const {
        return {
            {x-1, y-1}, {x, y-1}, {x+1, y-1},
            {x-1, y},             {x+1, y},
            {x-1, y+1}, {x, y+1}, {x+1, y+1}
        };
    }
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator<(const Position& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};
```

### CellState Component Details
```cpp
// In components/cell_state.hpp
#pragma once
#include <cstdint>

struct CellState {
    bool alive;
    uint32_t generation;
    uint8_t neighbor_count;
    
    CellState(bool alive = false, uint32_t generation = 0) 
        : alive(alive), generation(generation), neighbor_count(0) {}
    
    // Conway's Game of Life rules
    bool should_survive() const {
        return alive && (neighbor_count == 2 || neighbor_count == 3);
    }
    
    bool should_be_born() const {
        return !alive && neighbor_count == 3;
    }
    
    void update_generation() {
        generation++;
    }
};
```

### EnTT Registry Usage
```cpp
// In main.cpp or registry setup
#include <entt/entt.hpp>
#include "components/position.hpp"
#include "components/cell_state.hpp"

entt::registry registry;

// Create entity with components
auto entity = registry.create();
registry.emplace<Position>(entity, 10, 20);
registry.emplace<CellState>(entity, true, 0);

// Access components
auto& pos = registry.get<Position>(entity);
auto& state = registry.get<CellState>(entity);
```

## Next Steps
Upon completion, the following tasks will be available:
1. implement-entt-systems.md - Game logic systems
2. implement-entt-api.md - gRPC API server
3. entt-testing-framework.md - Comprehensive testing

## Success Indicators
- Components compile and link successfully
- EnTT registry integration works
- Game logic rules implemented correctly
- Component tests pass
- Memory usage meets efficiency requirements
- Ready for system implementation