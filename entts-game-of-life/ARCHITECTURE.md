# Architecture Documentation

This document outlines the technical architecture for the Game of Life implementation using EnTT.

## ECS Design with EnTT

### Core Philosophy
- **Entities represent live cells only**: Dead cells are not entities, reducing memory usage for sparse grids
- **Components are pure data**: No behavior, only state
- **Systems contain all logic**: Pure functions operating on component data
- **Performance-first**: Leverage EnTT's cache-friendly storage

### Component Design

#### Position Component
```cpp
struct Position {
    std::int32_t x, y;
};
```
- Represents cell coordinates in the grid
- Used as primary key for spatial queries
- Optimized for hash map lookups

#### CellState Component
```cpp
struct CellState {
    bool alive;
    std::uint8_t neighbor_count;
};
```
- Tracks current life state
- Caches neighbor count for performance
- Updated each generation

#### Generation Component
```cpp
struct Generation {
    std::uint64_t birth_generation;
    std::uint64_t last_updated;
};
```
- Tracks entity lifecycle
- Enables temporal queries
- Supports rollback functionality

### System Architecture

#### NeighborSystem
- **Purpose**: Calculate neighbor counts for all cells
- **Strategy**: Use spatial hash map for O(1) neighbor lookups
- **Optimization**: Only process cells and their immediate neighbors

#### LifecycleSystem
- **Purpose**: Apply Game of Life rules
- **Strategy**: Create/destroy entities based on rules
- **Optimization**: Batch entity operations for performance

#### RenderSystem
- **Purpose**: Output current state to console/file
- **Strategy**: Iterate entities in spatial order
- **Optimization**: Sparse rendering for large grids

### Sparse Representation Strategy

#### Memory Layout
```
Dense Grid (1000x1000):     1,000,000 cells stored
Sparse Grid (1% density):      10,000 entities stored
Memory reduction:                   99% less memory
```

#### Spatial Indexing
- **Hash Map**: `std::unordered_map<Position, entt::entity>`
- **Neighbor Queries**: Check 8 adjacent positions in O(1)
- **Iteration**: Process only living cells and their neighbors

#### Entity Lifecycle
1. **Birth**: Create entity when cell becomes alive
2. **Life**: Update components each generation
3. **Death**: Destroy entity when cell dies
4. **Cleanup**: Remove orphaned spatial references

## Configuration System Design

### JSON Schema
```json
{
  "grid": {
    "width": 1000,
    "height": 1000,
    "initial_pattern": "glider_gun"
  },
  "simulation": {
    "max_generations": 1000,
    "auto_pause_on_stable": true
  },
  "performance": {
    "target_fps": 60,
    "memory_limit_mb": 100
  }
}
```

### Configuration Loading
- **Validation**: JSON schema validation on load
- **Defaults**: Fallback values for missing keys
- **Hot Reload**: Support runtime configuration updates

### Pattern Library
- **Built-in Patterns**: Glider, gun, oscillators
- **Custom Patterns**: Load from RLE or plaintext files
- **Pattern Registry**: Named pattern lookup system

## Unity Integration Strategy

### Shared Library Design

#### C API Wrapper
```c
// Core simulation functions
GOL_HANDLE* gol_create(const char* config_json);
void gol_step(GOL_HANDLE* handle);
void gol_destroy(GOL_HANDLE* handle);

// State query functions
int gol_get_living_cells(GOL_HANDLE* handle, int* x_coords, int* y_coords);
bool gol_is_cell_alive(GOL_HANDLE* handle, int x, int y);

// Configuration functions
void gol_set_config(GOL_HANDLE* handle, const char* config_json);
char* gol_get_config(GOL_HANDLE* handle);
```

#### Memory Management
- **Handle-based API**: Opaque pointers hide C++ complexity
- **Manual lifetime**: Unity controls creation/destruction
- **Buffer management**: Unity provides arrays for bulk data transfer

#### Error Handling
- **Return codes**: Standard integer error codes
- **Error strings**: Detailed error messages available via API
- **Graceful degradation**: Safe fallbacks for invalid states

### Unity Package Structure
```
Packages/com.studio.gameoflife/
├── Runtime/
│   ├── Scripts/
│   │   ├── GameOfLifeSimulation.cs
│   │   └── GameOfLifeRenderer.cs
│   ├── Plugins/
│   │   ├── Windows/
│   │   │   └── game_of_life.dll
│   │   ├── macOS/
│   │   │   └── libgame_of_life.dylib
│   │   └── Linux/
│   │       └── libgame_of_life.so
│   └── package.json
├── Editor/
│   └── Scripts/
│       └── GameOfLifeEditor.cs
└── Samples~/
    └── BasicExample/
        └── BasicGameOfLife.unity
```

### Integration Patterns

#### Data Flow
1. **Unity → C++**: Configuration via JSON strings
2. **C++ → Unity**: Cell positions via coordinate arrays
3. **Unity**: Visualization using received data
4. **Performance**: Minimize P/Invoke calls

#### Threading Model
- **Main Thread**: Unity update loop calls C++ step function
- **C++ Internal**: Single-threaded for deterministic behavior
- **Future**: Job system integration for parallel processing

## Performance Considerations

### Memory Optimization

#### Component Storage
- **SoA Layout**: EnTT provides cache-friendly component storage
- **Minimal Components**: Only essential data in components
- **Memory Pools**: Reuse destroyed entities to reduce allocations

#### Spatial Efficiency
- **Sparse Storage**: Only store living cells
- **Neighbor Caching**: Pre-calculate frequently accessed data
- **Batch Operations**: Group entity creation/destruction

### Computation Optimization

#### Algorithm Complexity
- **Neighbor Calculation**: O(living_cells) instead of O(grid_size)
- **Rule Application**: O(active_region) for stable patterns
- **Spatial Queries**: O(1) hash map lookups

#### Cache Performance
- **Component Iteration**: Sequential access patterns
- **Spatial Locality**: Group nearby cells in memory
- **Prefetching**: Predictable access patterns

### Scalability Targets

| Grid Size | Living Cells | Memory Usage | Target FPS |
|-----------|--------------|--------------|------------|
| 100x100   | 5,000        | 1 MB         | 1000+      |
| 1,000x1,000| 50,000       | 10 MB        | 100+       |
| 10,000x10,000| 500,000    | 100 MB       | 10+        |

### Profiling Strategy
- **Micro-benchmarks**: Individual system performance
- **Integration tests**: End-to-end simulation performance  
- **Memory profiling**: Track allocation patterns
- **Unity profiler**: Integration performance measurement

## Extension Points

### Custom Patterns
- **Pattern Interface**: Standardized pattern loading
- **Rule Variations**: Support different cellular automata
- **Interactive Editing**: Runtime pattern modification

### Rendering Backends
- **Console Output**: Text-based visualization
- **Image Export**: PNG/BMP file generation
- **Unity Integration**: Real-time visualization
- **Web Assembly**: Browser-based simulation

### Performance Scaling
- **Parallel Systems**: Multi-threaded generation updates
- **GPU Compute**: Shader-based rule application
- **Distributed**: Network-based large-grid simulation