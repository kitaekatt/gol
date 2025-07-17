# Implement EnTT Game Logic Systems

## Task Overview
**Priority**: High  
**Assigned Mode**: EnTT  
**Dependencies**: implement-entt-components.md completion
**Status**: Pending

## Coordination Metadata
**Task Classification**: Implementation-Specific
**Conflicts With**: implement-entt-components.md (dependency)
**Depends On**: implement-entt-components.md
**Modifies Files**: gol-entt/src/systems/*, gol-entt/include/systems/*
**Reads Files**: gol-entt/src/components/*, gol-bevy/src/systems/*
**Parallel Safe**: Yes (with other implementations)
**Estimated Duration**: 90 minutes
**File Lock Priority**: Medium

## Objective
Implement the game logic systems for Conway's Game of Life using EnTT's system architecture, following the functional specification established by the Bevy implementation.

## Context
The Bevy implementation uses three core systems: neighbor counting, cell lifecycle, and cleanup. This task implements equivalent systems using EnTT's view and system patterns.

## Requirements

### 1. Core Systems
Based on the Bevy reference implementation:

#### Neighbor Counting System
- Count live neighbors for each cell
- Update neighbor_count field in CellState
- Optimize for sparse representation
- Handle edge cases and boundaries

#### Cell Lifecycle System
- Apply Conway's Game of Life rules
- Create new cells when conditions are met
- Update existing cell states
- Manage generation counter

#### Cleanup System
- Remove dead cells to maintain sparse representation
- Optimize memory usage
- Clean up orphaned components
- Maintain performance under load

### 2. System Architecture
- Implement systems as functions taking EnTT registry
- Use EnTT views for efficient entity queries
- Maintain proper system execution order
- Handle system dependencies and scheduling

### 3. Performance Optimization
- Minimize component access overhead
- Use efficient data structures
- Optimize neighbor calculation
- Maintain <1KB per 1000 live cells target

## Implementation Steps

### Phase 1: System Structure
1. Create system header files and function signatures
2. Implement basic EnTT view queries
3. Set up system execution framework
4. Create system scheduling mechanism

### Phase 2: Neighbor Counting System
1. Implement neighbor position calculation
2. Count live neighbors for each cell
3. Update CellState neighbor_count field
4. Optimize for sparse cell distribution

### Phase 3: Cell Lifecycle System
1. Apply Conway's Game of Life rules
2. Create new entities for born cells
3. Update existing cell states
4. Handle state transitions properly

### Phase 4: Cleanup System
1. Remove dead entities and components
2. Optimize memory usage patterns
3. Maintain sparse representation
4. Handle cleanup scheduling

## Acceptance Criteria

### System Implementation
- [ ] Neighbor counting system functional
- [ ] Cell lifecycle system applies Conway's rules
- [ ] Cleanup system maintains sparse representation
- [ ] Proper system execution order

### Game Logic
- [ ] Conway's Game of Life rules correctly implemented
- [ ] All standard patterns behave correctly
- [ ] State transitions work properly
- [ ] Neighbor counting is accurate

### Performance
- [ ] Memory usage <1KB per 1000 live cells
- [ ] Efficient system execution
- [ ] Optimized component access
- [ ] Scalable to large simulations

### Integration
- [ ] Systems work with EnTT registry
- [ ] Proper component lifecycle management
- [ ] System scheduling functional
- [ ] Ready for API integration

## Reference Implementation
Use the Bevy implementation as functional reference:
- **Systems**: /mnt/c/dev/gol/gol-bevy/src/systems/
- **Game Logic**: Conway's Game of Life rules
- **Performance**: Memory and speed requirements

## Technical Specifications

### System Function Signatures
```cpp
// In systems/neighbor_system.hpp
#pragma once
#include <entt/entt.hpp>

class NeighborSystem {
public:
    static void update(entt::registry& registry);
    
private:
    static void count_neighbors_for_entity(entt::registry& registry, entt::entity entity);
    static int count_live_neighbors(entt::registry& registry, const Position& pos);
};
```

### Neighbor Counting Implementation
```cpp
// In systems/neighbor_system.cpp
#include "neighbor_system.hpp"
#include "../components/position.hpp"
#include "../components/cell_state.hpp"

void NeighborSystem::update(entt::registry& registry) {
    auto view = registry.view<Position, CellState>();
    
    for (auto entity : view) {
        count_neighbors_for_entity(registry, entity);
    }
}

void NeighborSystem::count_neighbors_for_entity(entt::registry& registry, entt::entity entity) {
    auto& pos = registry.get<Position>(entity);
    auto& state = registry.get<CellState>(entity);
    
    state.neighbor_count = count_live_neighbors(registry, pos);
}

int NeighborSystem::count_live_neighbors(entt::registry& registry, const Position& pos) {
    int count = 0;
    for (const auto& neighbor_pos : pos.get_neighbors()) {
        auto view = registry.view<Position, CellState>();
        for (auto entity : view) {
            auto& entity_pos = registry.get<Position>(entity);
            auto& entity_state = registry.get<CellState>(entity);
            
            if (entity_pos == neighbor_pos && entity_state.alive) {
                count++;
            }
        }
    }
    return count;
}
```

### Lifecycle System Implementation
```cpp
// In systems/lifecycle_system.hpp
#pragma once
#include <entt/entt.hpp>
#include <vector>

class LifecycleSystem {
public:
    static void update(entt::registry& registry);
    
private:
    static void process_existing_cells(entt::registry& registry);
    static void create_new_cells(entt::registry& registry);
    static std::vector<Position> find_birth_positions(entt::registry& registry);
};
```

### Cleanup System Implementation
```cpp
// In systems/cleanup_system.hpp
#pragma once
#include <entt/entt.hpp>

class CleanupSystem {
public:
    static void update(entt::registry& registry);
    
private:
    static void remove_dead_entities(entt::registry& registry);
    static void optimize_memory(entt::registry& registry);
};
```

### System Execution Framework
```cpp
// In main.cpp or system manager
#include "systems/neighbor_system.hpp"
#include "systems/lifecycle_system.hpp"
#include "systems/cleanup_system.hpp"

class SystemManager {
public:
    static void update_generation(entt::registry& registry) {
        // Execute systems in proper order
        NeighborSystem::update(registry);
        LifecycleSystem::update(registry);
        CleanupSystem::update(registry);
    }
};
```

## Performance Considerations

### Memory Optimization
- Use sparse storage for entities
- Minimize component size
- Optimize neighbor calculation
- Batch entity operations

### Computational Efficiency
- Use EnTT views for fast iteration
- Cache neighbor positions
- Minimize registry lookups
- Optimize hot paths

### Scalability
- Handle large simulations efficiently
- Maintain consistent performance
- Optimize for typical Game of Life patterns
- Scale to thousands of live cells

## Next Steps
Upon completion, the following tasks will be available:
1. implement-entt-api.md - gRPC API server
2. entt-testing-framework.md - Comprehensive testing
3. entt-performance-optimization.md - Performance tuning

## Success Indicators
- Systems compile and execute correctly
- Conway's Game of Life rules work properly
- Performance meets requirements
- Memory usage optimized
- Integration with components successful
- Ready for API server implementation