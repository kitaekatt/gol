# Claude Development Guidelines

This document provides essential context for AI-assisted development of the Bevy Game of Life project.

## Tech Stack

- **Language**: Rust (latest stable)
- **Game Engine**: Bevy (latest stable version)
- **Serialization**: serde with serde_json for JSON configuration
- **Testing**: Built-in Rust testing framework with cargo test
- **Build System**: Cargo with workspace configuration

## Project Structure

```
bevy-game-of-life/
├── src/
│   ├── main.rs              # Application entry point
│   ├── lib.rs               # Library root
│   ├── components/          # ECS components
│   │   ├── mod.rs
│   │   ├── cell.rs          # Cell-related components
│   │   └── grid.rs          # Grid and position components
│   ├── systems/             # Bevy systems
│   │   ├── mod.rs
│   │   ├── game_of_life.rs  # Core Game of Life logic
│   │   ├── rendering.rs     # Visualization systems
│   │   └── input.rs         # User input handling
│   ├── resources/           # Bevy resources
│   │   ├── mod.rs
│   │   ├── config.rs        # Configuration management
│   │   └── grid_state.rs    # Grid state management
│   └── plugins/             # Bevy plugins
│       ├── mod.rs
│       └── game_of_life.rs  # Main plugin
├── assets/                  # Game assets
│   ├── patterns/            # Pattern JSON files
│   └── textures/            # Cell textures/sprites
├── tests/                   # Integration tests
│   ├── game_logic.rs
│   └── pattern_loading.rs
├── benches/                 # Performance benchmarks
│   └── grid_performance.rs
├── examples/                # Example configurations
│   └── basic_patterns.rs
└── docs/                    # Additional documentation
    └── patterns.md
```

## Naming Conventions

### Components
- Use descriptive names: `CellState`, `GridPosition`, `NeighborCount`
- Derive Debug, Clone when appropriate
- Follow Bevy component patterns

### Systems
- Use verb-noun pattern: `update_cell_states`, `render_grid`, `handle_input`
- Group related systems in modules
- Use clear function signatures with proper system parameters

### Resources
- Prefix with project context: `GameConfig`, `GridState`, `SimulationTimer`
- Implement Default when sensible
- Use serde derives for serializable resources

## Build Commands

```bash
# Development build
cargo build

# Release build with optimizations
cargo build --release

# Run application
cargo run

# Run with release optimizations
cargo run --release

# Run all tests
cargo test

# Run specific test module
cargo test game_logic

# Run benchmarks
cargo bench

# Check code without building
cargo check

# Format code
cargo fmt

# Lint code
cargo clippy

# Generate documentation
cargo doc --open
```

## Performance Requirements

### Memory Efficiency
- Use sparse grid representation (only store live cells)
- Prefer `Vec` over `HashMap` for iteration-heavy operations
- Minimize entity component data size
- Use object pooling for frequently spawned/despawned entities

### Computational Efficiency
- Leverage Bevy's parallel system execution
- Minimize neighbor count recalculations
- Use spatial partitioning for large grids
- Profile with `cargo bench` before optimizing

### Frame Rate Targets
- Maintain 60 FPS for grids up to 1000x1000
- Graceful degradation for larger grids
- Configurable update rates independent of render rate

## Coding Style

### Error Handling
- Use `Result<T, E>` for fallible operations
- Prefer `expect()` with descriptive messages over `unwrap()`
- Handle configuration loading errors gracefully
- Log errors using Bevy's logging system

### Documentation
- Document all public APIs with rustdoc
- Include examples in documentation
- Document complex algorithms and performance considerations
- Maintain up-to-date README and architecture docs

### Testing Approach
- Unit tests for isolated logic (rule calculations, pattern parsing)
- Integration tests for system interactions
- Property-based testing for Game of Life rules
- Benchmark tests for performance regression detection

## TDD Workflow Expectations

### Test-First Development
1. Write failing test describing desired behavior
2. Implement minimal code to make test pass
3. Refactor while keeping tests green
4. Repeat for next feature

### Test Categories
- **Unit Tests**: Individual component/system behavior
- **Integration Tests**: System interaction and data flow
- **Property Tests**: Game of Life mathematical properties
- **Performance Tests**: Benchmark critical paths

### Test Naming
- Use descriptive test names: `test_cell_with_two_neighbors_survives`
- Group related tests in modules
- Use `#[should_panic]` for error condition tests

### Bevy Testing Patterns
- Use `App::new()` for system testing
- Mock time progression with `Time` resource
- Test component state changes through queries
- Verify entity spawning/despawning in generation transitions

## Configuration Management

### JSON Schema
- Use serde for automatic serialization/deserialization
- Provide default configurations
- Validate configuration on load
- Support hot-reloading during development

### Pattern Files
- Store patterns as JSON with metadata
- Include pattern name, description, and author
- Support both absolute and relative coordinates
- Validate pattern bounds against grid size

## Dependencies Philosophy

- Prefer Bevy ecosystem crates when available
- Minimize external dependencies
- Choose mature, well-maintained crates
- Document rationale for each major dependency