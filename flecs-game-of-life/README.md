# FLECS Game of Life

A high-performance implementation of Conway's Game of Life using the FLECS Entity Component System, designed as a proof-of-concept for Unity integration.

## Project Overview

This project demonstrates advanced ECS patterns and performance optimization techniques by implementing Conway's Game of Life in C++ using the FLECS ECS framework. The primary goal is to create a foundation that can be seamlessly integrated into Unity as a native plugin, showcasing how modern ECS architectures can bridge console applications and game engines.

### Goals

- **Console Implementation**: Create a fully functional Game of Life with configurable parameters and real-time visualization
- **Performance Focus**: Optimize for large grid sizes (10,000+ cells) with minimal memory footprint
- **Unity Integration**: Design as a shared library with C API for Unity native plugin integration
- **Proof of Concept**: Demonstrate FLECS ECS patterns applicable to larger game development projects

## Architecture Decisions

### ECS with FLECS
- **Sparse Entity Representation**: Only live cells exist as entities, dramatically reducing memory usage
- **Component-Based Logic**: Separate components for position, state, and neighbor counting
- **System-Driven Updates**: Dedicated systems for neighbor calculation, rule application, and lifecycle management
- **Query-Optimized**: Leverage FLECS queries for efficient spatial operations

### Performance-Focused Design
- **Memory Efficiency**: Sparse representation eliminates empty cell storage overhead
- **Computational Optimization**: Neighbor calculation only for active regions
- **Configurable Limits**: Adjustable grid boundaries and performance parameters
- **Profiling Integration**: Built-in timing and memory usage tracking

### Shared Library Approach
- **C API Layer**: Clean interface for Unity P/Invoke integration
- **State Management**: Serializable game state for save/load functionality
- **Event System**: Callback mechanisms for Unity integration points
- **Thread Safety**: Design considerations for multi-threaded environments

## Conway's Game of Life Rules

This implementation follows the standard rules of Conway's Game of Life. For the complete specification and background, see the [Wikipedia article on Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life).

## Development Phases

### Phase 1: Core ECS Implementation
- [ ] FLECS integration and basic ECS setup
- [ ] Entity/component design for cells and grid state
- [ ] Core Game of Life logic systems
- [ ] Console visualization and basic I/O

### Phase 2: Performance Optimization
- [ ] Sparse representation optimization
- [ ] Neighbor calculation efficiency improvements
- [ ] Memory usage profiling and optimization
- [ ] Benchmark suite for performance validation

### Phase 3: Configuration and Persistence
- [ ] JSON-based configuration system
- [ ] Save/load functionality for game states
- [ ] Pattern library integration
- [ ] Command-line interface enhancements

### Phase 4: Unity Integration Preparation
- [ ] C API layer development
- [ ] Shared library build configuration
- [ ] Unity plugin interface design
- [ ] Cross-platform compatibility testing

### Phase 5: Unity Plugin Development
- [ ] Unity package creation
- [ ] Native plugin integration
- [ ] Unity-specific visualization
- [ ] Performance comparison and validation

## Project Structure

```
flecs-game-of-life/
├── src/                    # Source code
│   ├── core/              # Core ECS simulation library
│   ├── console/           # Console application frontend
│   ├── unity/             # Unity plugin C API layer
│   ├── config/            # Configuration management
│   └── utils/             # Utility functions
├── include/               # Public headers
│   └── flecs_gol/         # Library public interface
├── tests/                 # Unit and integration tests
│   ├── unit/              # Unit tests
│   ├── integration/       # Integration tests
│   └── performance/       # Performance benchmarks
├── examples/              # Example configurations and patterns
│   ├── patterns/          # Game of Life patterns (JSON)
│   └── configs/           # Example configurations
├── config/                # Default configuration files
├── unity/                 # Unity package and integration
├── docs/                  # Additional documentation
├── CMakeLists.txt         # Build configuration
├── vcpkg.json             # Dependency manifest
└── README.md              # This file
```

## Getting Started

Refer to `CLAUDE.md` for detailed development setup instructions and `ARCHITECTURE.md` for in-depth technical documentation.

## License

[License to be determined]