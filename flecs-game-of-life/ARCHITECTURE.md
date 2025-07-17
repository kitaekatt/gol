# FLECS Game of Life - Architecture Documentation

## ECS Design with FLECS

### Entity-Component Architecture

#### Entities
- **LiveCell**: Represents an active cell in the game grid
- **DeadCell**: Temporary entities for cells transitioning from alive to dead
- **Grid**: Singleton entity containing global game state and configuration

#### Components

```cpp
// Position component for spatial indexing
struct Position {
    int32_t x, y;
    
    // Hash function for spatial queries
    size_t hash() const { return (size_t(x) << 32) | size_t(y); }
};

// Cell lifecycle state
struct CellState {
    uint8_t neighborCount = 0;
    bool willLive = false;  // Computed during rule evaluation
};

// Grid boundaries and metadata
struct GridConfig {
    int32_t minX, maxX, minY, maxY;
    uint32_t generation = 0;
    uint32_t liveCellCount = 0;
};

// Performance tracking
struct PerformanceMetrics {
    std::chrono::microseconds lastUpdateTime;
    size_t memoryUsage;
    uint32_t entityCount;
};
```

#### Sparse Representation Strategy

The core architectural decision is to use sparse representation where only live cells exist as entities:

**Benefits:**
- Memory usage scales with O(live_cells) instead of O(grid_size²)
- Cache-friendly iteration over active cells only
- Natural spatial locality for neighbor calculations
- Efficient entity lifecycle management

**Implementation:**
- FLECS spatial indexing for O(1) position lookups
- Hash-based neighbor finding using Position component
- Batch entity creation/destruction for performance
- Query-based systems operating only on relevant entities

### System Architecture

#### Core Systems

1. **NeighborCountSystem**
   - Iterates over all live cells
   - Counts neighbors for each cell and its dead neighbors
   - Updates CellState.neighborCount
   - Creates temporary entities for potentially born cells

2. **RuleEvaluationSystem**
   - Applies Conway's Game of Life rules
   - Sets CellState.willLive based on neighbor count
   - Prepares entities for lifecycle transitions

3. **LifecycleSystem**
   - Creates new LiveCell entities for births
   - Destroys entities for deaths
   - Updates GridConfig statistics
   - Manages entity recycling for performance

4. **RenderSystem** (Console)
   - Queries live cells within viewport bounds
   - Generates ASCII representation
   - Handles camera/viewport management
   - Updates performance displays

#### System Execution Order

```
Frame N:
1. NeighborCountSystem    - Calculate neighbors
2. RuleEvaluationSystem   - Apply Game of Life rules
3. LifecycleSystem        - Update entity states
4. RenderSystem           - Display current state
5. PerformanceSystem      - Update metrics
```

### Spatial Indexing and Queries

#### Position-Based Lookups

FLECS queries enable efficient spatial operations:

```cpp
// Find all cells in region
auto query = world.query<Position>()
    .with([](flecs::iter& it, Position* pos) {
        for (auto i : it) {
            if (pos[i].x >= minX && pos[i].x <= maxX &&
                pos[i].y >= minY && pos[i].y <= maxY) {
                // Process cell
            }
        }
    });

// Find cell at specific position (using hash map for O(1))
std::unordered_map<size_t, flecs::entity> positionIndex;
```

#### Neighbor Calculation Optimization

```cpp
// For each live cell, check 8 neighbors
static const std::array<std::pair<int, int>, 8> NEIGHBOR_OFFSETS = {{
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1},          { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
}};

// Batch neighbor lookup using spatial index
for (const auto& offset : NEIGHBOR_OFFSETS) {
    Position neighborPos{pos.x + offset.first, pos.y + offset.second};
    // O(1) lookup in position index
}
```

## Configuration System Design

### JSON Schema

```json
{
  "grid": {
    "boundaries": {
      "minX": -1000, "maxX": 1000,
      "minY": -1000, "maxY": 1000
    },
    "initialPattern": "patterns/glider.json"
  },
  "performance": {
    "targetFPS": 60,
    "maxEntities": 1000000,
    "enableProfiling": true
  },
  "rendering": {
    "viewportWidth": 80,
    "viewportHeight": 24,
    "liveChar": "#",
    "deadChar": " "
  },
  "unity": {
    "apiVersion": "1.0",
    "threadSafe": true,
    "callbackFrequency": 16
  }
}
```

### Configuration Management

```cpp
class ConfigManager {
public:
    // Load configuration from JSON file
    static std::expected<Config, Error> loadFromFile(const std::string& path);
    
    // Save current configuration
    static std::expected<void, Error> saveToFile(const Config& config, const std::string& path);
    
    // Validate configuration constraints
    static std::expected<void, Error> validate(const Config& config);
    
    // Apply configuration to ECS world
    static void applyToWorld(flecs::world& world, const Config& config);
};
```

### Pattern Library Integration

Pre-defined patterns stored as JSON with metadata:

```json
{
  "name": "Glider",
  "description": "Simple moving pattern",
  "period": 4,
  "displacement": [1, 1],
  "boundingBox": {"width": 3, "height": 3},
  "cells": [
    {"x": 1, "y": 0},
    {"x": 2, "y": 1},
    {"x": 0, "y": 2},
    {"x": 1, "y": 2},
    {"x": 2, "y": 2}
  ]
}
```

## Unity Integration Strategy

### Shared Library Design

#### C API Layer

```c
// Core API functions
extern "C" {
    // World management
    GOL_Handle* gol_create_world(const char* configJson);
    void gol_destroy_world(GOL_Handle* handle);
    
    // Simulation control
    void gol_step(GOL_Handle* handle);
    void gol_run_for_generations(GOL_Handle* handle, uint32_t count);
    void gol_pause(GOL_Handle* handle);
    void gol_reset(GOL_Handle* handle);
    
    // State queries
    uint32_t gol_get_live_cell_count(const GOL_Handle* handle);
    uint32_t gol_get_generation(const GOL_Handle* handle);
    
    // Cell data access
    GOL_CellData* gol_get_cells_in_region(const GOL_Handle* handle, 
                                          int32_t minX, int32_t minY,
                                          int32_t maxX, int32_t maxY,
                                          uint32_t* cellCount);
    void gol_free_cell_data(GOL_CellData* data);
    
    // Configuration
    int gol_load_pattern(GOL_Handle* handle, const char* patternJson);
    int gol_save_state(const GOL_Handle* handle, const char* filename);
    int gol_load_state(GOL_Handle* handle, const char* filename);
}
```

#### Thread Safety Considerations

- **Read-Only Access**: Unity main thread can safely query state
- **Update Thread**: Dedicated thread for ECS simulation updates
- **Synchronization**: Double-buffering for state queries during updates
- **Memory Management**: RAII with clear ownership boundaries

#### Unity Plugin Interface

```csharp
[DllImport("flecs_gol")]
public static extern IntPtr gol_create_world(string configJson);

[DllImport("flecs_gol")]
public static extern void gol_step(IntPtr handle);

[DllImport("flecs_gol")]
public static extern uint gol_get_live_cell_count(IntPtr handle);

public class GameOfLife : MonoBehaviour {
    private IntPtr nativeHandle;
    
    void Start() {
        string config = LoadConfigFromResources();
        nativeHandle = gol_create_world(config);
    }
    
    void Update() {
        if (Time.deltaTime >= targetFrameTime) {
            gol_step(nativeHandle);
            UpdateVisualization();
        }
    }
}
```

### Data Marshaling Strategy

#### Efficient Cell Data Transfer

```cpp
// Optimized data structure for Unity transfer
struct GOL_CellData {
    int32_t x, y;
    uint8_t state;  // 0=dead, 1=alive, 2=dying, 3=being_born
};

// Batch data transfer to minimize P/Invoke overhead
std::vector<GOL_CellData> getCellsInRegion(
    const flecs::world& world,
    int32_t minX, int32_t minY,
    int32_t maxX, int32_t maxY) {
    
    std::vector<GOL_CellData> result;
    auto query = world.query<Position, CellState>();
    
    query.each([&](Position pos, CellState state) {
        if (pos.x >= minX && pos.x <= maxX && 
            pos.y >= minY && pos.y <= maxY) {
            result.push_back({pos.x, pos.y, 1});
        }
    });
    
    return result;
}
```

## Performance Considerations

### Memory Optimization

#### Entity Pooling
- Pre-allocate entity pools to avoid runtime allocation
- Recycle dead cell entities for new births
- Minimize component memory footprint

#### Spatial Locality
- Group entities by spatial regions
- Cache-friendly iteration patterns
- Minimize pointer chasing in critical paths

#### Memory Mapping
```cpp
// Memory layout optimization
struct alignas(64) OptimizedCell {  // Cache line aligned
    Position pos;        // 8 bytes
    CellState state;     // 2 bytes
    // 54 bytes padding for cache alignment
};
```

### Computational Optimization

#### Parallel Processing
- Multi-threaded neighbor calculation
- SIMD optimization for batch operations
- Task-based parallelism with FLECS threading

#### Algorithmic Improvements
- Incremental updates for stable regions
- Spatial partitioning for large grids
- Early termination for oscillating patterns

#### Profiling Integration
```cpp
class PerformanceProfiler {
    std::chrono::high_resolution_clock::time_point frameStart;
    std::unordered_map<std::string, std::chrono::microseconds> systemTimes;
    
public:
    void beginFrame();
    void beginSystem(const std::string& name);
    void endSystem(const std::string& name);
    void endFrame();
    
    const auto& getMetrics() const { return systemTimes; }
};
```

### Scalability Targets

- **Small Grids** (1k cells): >200 FPS on single core
- **Medium Grids** (100k cells): >60 FPS with multi-threading
- **Large Grids** (1M+ cells): >30 FPS with optimized algorithms
- **Memory Usage**: <1KB per 1000 live cells
- **Unity Integration**: <1ms update budget per frame