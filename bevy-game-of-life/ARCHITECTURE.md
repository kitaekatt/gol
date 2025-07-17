# Architecture Design

This document details the technical architecture and design decisions for the Bevy Game of Life implementation.

## Bevy ECS Design

### Entity-Component Architecture

#### Cell Entity Design
Each live cell in the Game of Life is represented as a Bevy entity with the following components:

```rust
// Core cell components
struct CellState {
    alive: bool,
    age: u32,  // Generations alive (for visualization)
}

struct GridPosition {
    x: i32,
    y: i32,
}

struct NeighborCount {
    count: u8,  // Cached neighbor count for performance
    dirty: bool, // Needs recalculation
}

// Rendering components (from Bevy)
// - Transform: World position for rendering
// - Sprite: Visual representation
// - Visibility: Culling and LOD
```

#### Sparse Grid Representation
- **Live cells only**: Only cells that are alive exist as entities
- **Memory efficiency**: Reduces memory usage for sparse patterns
- **Dynamic spawning**: Cells are spawned/despawned each generation
- **Spatial queries**: Use Bevy's spatial partitioning for neighbor detection

### System Architecture

#### Core Systems Execution Order

1. **Input Systems** (`PreUpdate`)
   - `handle_pause_toggle`: Space bar to pause/unpause
   - `handle_step_input`: Step-by-step execution
   - `handle_zoom_pan`: Camera controls
   - `handle_pattern_loading`: Load patterns from files

2. **Simulation Systems** (`Update`)
   - `update_neighbor_counts`: Calculate neighbors for all cells
   - `apply_game_of_life_rules`: Determine next generation state
   - `spawn_new_cells`: Create entities for newly alive cells
   - `despawn_dead_cells`: Remove entities for dead cells

3. **Rendering Systems** (`PostUpdate`)
   - `update_cell_transforms`: Position cells in world space
   - `update_cell_colors`: Color cells based on age/state
   - `update_grid_lines`: Draw grid overlay (optional)

#### System Dependencies
```rust
// Neighbor calculation must happen before rule application
update_neighbor_counts.before(apply_game_of_life_rules)

// Rule application must happen before entity spawn/despawn
apply_game_of_life_rules
    .before(spawn_new_cells)
    .before(despawn_dead_cells)

// Entity changes must complete before rendering updates
spawn_new_cells.before(update_cell_transforms)
despawn_dead_cells.before(update_cell_transforms)
```

## Configuration System Design

### JSON Configuration Schema

```json
{
  "grid": {
    "width": 100,
    "height": 100,
    "wrap_edges": false
  },
  "simulation": {
    "steps_per_second": 10,
    "auto_start": true,
    "max_generations": null
  },
  "rendering": {
    "cell_size": 8.0,
    "grid_lines": true,
    "color_scheme": "classic",
    "smooth_transitions": true
  },
  "initial_pattern": {
    "type": "file",
    "path": "assets/patterns/glider.json"
  }
}
```

### Configuration Loading Strategy

```rust
// Resource for runtime configuration
#[derive(Resource, Serialize, Deserialize)]
struct GameConfig {
    grid: GridConfig,
    simulation: SimulationConfig,
    rendering: RenderingConfig,
    initial_pattern: PatternConfig,
}

// Hot-reload support
fn reload_config_system(
    mut config: ResMut<GameConfig>,
    // File watcher integration
) {
    // Reload and validate configuration
    // Apply changes to running simulation
}
```

### Pattern File Format

```json
{
  "name": "Glider",
  "description": "Simple moving pattern",
  "author": "John Conway",
  "size": { "width": 3, "height": 3 },
  "cells": [
    {"x": 1, "y": 0},
    {"x": 2, "y": 1},
    {"x": 0, "y": 2},
    {"x": 1, "y": 2},
    {"x": 2, "y": 2}
  ]
}
```

## Real-Time Rendering Strategy

### 2D Sprite-Based Rendering

#### Cell Visualization
- **Sprite components**: Each cell entity has a `Sprite` component
- **Instanced rendering**: Leverage Bevy's sprite batching for performance
- **Texture atlas**: Single texture with multiple cell states/ages
- **Color modulation**: Tint sprites based on cell age or state

#### Camera and Viewport
```rust
// Orthographic camera for 2D grid view
fn setup_camera(mut commands: Commands) {
    commands.spawn(Camera2dBundle {
        projection: OrthographicProjection {
            scale: 1.0,
            ..default()
        },
        ..default()
    });
}

// Zoom and pan controls
fn camera_controls(
    mut camera_query: Query<&mut OrthographicProjection, With<Camera>>,
    input: Res<Input<KeyCode>>,
    mouse: Res<Input<MouseButton>>,
) {
    // Handle zoom (mouse wheel)
    // Handle pan (mouse drag)
}
```

### Performance Optimizations

#### Viewport Culling
```rust
// Only render cells visible in camera viewport
fn cull_cells_system(
    camera_query: Query<(&Camera, &GlobalTransform)>,
    mut cell_query: Query<(&GridPosition, &mut Visibility)>,
    config: Res<GameConfig>,
) {
    // Calculate visible grid bounds
    // Set cell visibility based on viewport
}
```

#### Level of Detail (LOD)
- **Distance-based scaling**: Smaller cells at high zoom levels
- **Simplified rendering**: Solid colors instead of detailed sprites at distance
- **Batch updates**: Group cell state changes for efficient rendering

## Performance Considerations

### Memory Management

#### Entity Pooling
```rust
// Pool entities to avoid constant allocation
#[derive(Resource)]
struct CellEntityPool {
    available: Vec<Entity>,
    in_use: HashSet<Entity>,
}

fn spawn_cell_from_pool(
    pool: &mut CellEntityPool,
    commands: &mut Commands,
    position: GridPosition,
) -> Entity {
    // Reuse pooled entity or create new one
}
```

#### Component Data Layout
- **Pack components efficiently**: Minimize component size
- **Cache-friendly access**: Group related components together
- **Avoid unnecessary components**: Only add components when needed

### Computational Optimization

#### Spatial Partitioning
```rust
// Grid-based spatial lookup for neighbor queries
#[derive(Resource)]
struct SpatialGrid {
    cells: HashMap<(i32, i32), Entity>,
    chunk_size: i32,
}

fn get_neighbors(
    position: &GridPosition,
    spatial_grid: &SpatialGrid,
) -> Vec<Entity> {
    // Fast O(1) neighbor lookup
}
```

#### Parallel Processing
- **System parallelization**: Leverage Bevy's automatic system parallelization
- **Component queries**: Use parallel iterators for component updates
- **Thread-safe resources**: Design resources for concurrent access

#### Algorithm Complexity
- **Neighbor counting**: O(1) amortized with caching
- **Rule application**: O(n) where n is number of live cells
- **Entity management**: Batch operations for O(1) amortized spawning

### Scalability Targets

#### Grid Size Performance
- **Small grids** (< 100x100): 60+ FPS, all features enabled
- **Medium grids** (100x500): 30+ FPS, basic optimizations
- **Large grids** (500x1000): 15+ FPS, aggressive culling and LOD
- **Huge grids** (1000x1000+): Variable FPS, streaming/chunking

#### Memory Usage Guidelines
- **Target**: < 100MB for 1000x1000 sparse grid
- **Cell entity overhead**: ~64 bytes per live cell
- **Texture memory**: Shared atlas, minimal per-cell cost
- **System overhead**: Bevy scheduling and query caching

### Profiling and Benchmarking

#### Key Metrics
- **Frame time**: Total time per frame (16.67ms target for 60 FPS)
- **System time**: Time spent in each system
- **Memory usage**: Heap allocation and entity count
- **Generation time**: Time to compute one Game of Life step

#### Benchmark Targets
```rust
#[bench]
fn bench_large_grid_generation(b: &mut Bencher) {
    // 1000x1000 grid with 10% density
    // Target: < 50ms per generation
}

#[bench]
fn bench_neighbor_calculation(b: &mut Bencher) {
    // Neighbor counting for various patterns
    // Target: < 1ms for 10,000 cells
}
```