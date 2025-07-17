# Bevy Game of Life

A high-performance, real-time implementation of Conway's Game of Life using the Bevy ECS game engine.

## Project Overview

This project implements Conway's Game of Life as a native Rust application using Bevy's Entity Component System (ECS) architecture. The focus is on creating a performant, visually appealing cellular automaton with real-time visualization capabilities.

### Goals

- **Performance-First**: Leverage Bevy's ECS for efficient sparse grid representation and parallel processing
- **Real-Time Visualization**: Smooth 2D rendering with configurable frame rates and grid sizes
- **Configurable**: JSON-based configuration system for grid dimensions, initial patterns, and simulation parameters
- **Educational**: Clear, well-documented code demonstrating Bevy ECS patterns and game development concepts

## Architecture Decisions

### Bevy ECS Native Implementation
- **Entity-based cells**: Each live cell is represented as a Bevy entity with position and state components
- **Sparse representation**: Only live cells are stored as entities, reducing memory footprint for sparse patterns
- **System-based logic**: Game of Life rules implemented as Bevy systems for parallel execution
- **Component-driven design**: Cell state, position, and rendering data stored as separate components

### Performance Focus
- **Spatial partitioning**: Efficient neighbor queries using Bevy's transform and spatial systems
- **Batch processing**: Bulk entity spawning/despawning for generation transitions
- **Memory efficiency**: Sparse grid representation scales well with large, sparse patterns
- **Parallel execution**: Leverage Bevy's automatic system parallelization

### Technology Stack
- **Rust**: Memory safety and zero-cost abstractions
- **Bevy**: Modern ECS game engine with built-in rendering and windowing
- **Serde**: JSON serialization for configuration and pattern loading

## Game of Life Rules

This implementation follows the standard Conway's Game of Life rules. For a complete specification, see: [Conway's Game of Life - Wikipedia](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

## Development Phases

### Phase 1: Foundation (Current)
- [x] Project documentation and architecture design
- [ ] Basic Bevy application setup with window and rendering
- [ ] Core ECS components and systems structure
- [ ] Simple grid visualization

### Phase 2: Core Logic
- [ ] Game of Life rule implementation using Bevy systems
- [ ] Efficient neighbor counting algorithm
- [ ] Generation stepping and pause/play controls
- [ ] Basic patterns (blinker, glider, block)

### Phase 3: Configuration & Patterns
- [ ] JSON configuration system with serde
- [ ] Pattern loading from files
- [ ] Configurable grid dimensions and simulation speed
- [ ] Pattern library integration

### Phase 4: Enhanced Visualization
- [ ] Smooth cell animations and transitions
- [ ] Zoom and pan controls
- [ ] Color schemes and visual themes
- [ ] Performance metrics display

### Phase 5: Advanced Features
- [ ] Large grid optimization (viewport culling)
- [ ] Pattern editor and saving
- [ ] Statistics tracking (population, generations)
- [ ] Benchmarking and performance analysis

## Building and Running

```bash
# Build the project
cargo build

# Run with default configuration
cargo run

# Run tests
cargo test

# Run with optimizations
cargo run --release
```

## Contributing

This project follows Test-Driven Development (TDD) practices. See `CLAUDE.md` for detailed development guidelines and coding standards.

## License

This project is open source. License to be determined.