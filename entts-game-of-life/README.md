# Game of Life with EnTT

A high-performance Conway's Game of Life implementation using the EnTT Entity Component System, designed as a proof-of-concept for Unity integration.

## Project Overview

This project implements Conway's Game of Life as a console application with the primary goal of demonstrating how EnTT can be used to create a shared library suitable for Unity integration. The implementation focuses on performance, modularity, and clean architecture patterns.

### Goals

- **Primary**: Create a high-performance Game of Life simulation using EnTT ECS
- **Secondary**: Develop a shared library architecture for Unity integration
- **Proof-of-Concept**: Demonstrate ECS patterns in game engine integration scenarios

### Game Rules

This implementation follows Conway's Game of Life rules as specified in the [original mathematical definition](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life):

- Any live cell with 2-3 live neighbors survives
- Any dead cell with exactly 3 live neighbors becomes alive
- All other live cells die in the next generation
- All other dead cells stay dead

## Architecture Decisions

### Entity Component System (ECS)
- **EnTT**: Modern, header-only ECS library for optimal performance
- **Sparse representation**: Only living cells are entities, reducing memory footprint
- **Component-based**: Position, state, and neighbor data as separate components

### Performance Focus
- **Memory efficiency**: Sparse grid representation for large simulations
- **Cache-friendly**: Contiguous component storage via EnTT
- **Scalable**: Designed to handle grids up to 10,000x10,000 cells

### Shared Library Approach
- **C API**: Clean interface for Unity P/Invoke integration
- **State management**: JSON-based configuration persistence
- **Cross-platform**: Windows, macOS, and Linux support

## Development Phases

### Phase 1: Core Implementation (Weeks 1-2)
- [ ] Basic ECS architecture with EnTT
- [ ] Console-based visualization
- [ ] Core Game of Life logic
- [ ] JSON configuration system

### Phase 2: Performance Optimization (Week 3)
- [ ] Sparse grid optimization
- [ ] Benchmark suite
- [ ] Memory profiling and optimization
- [ ] Large grid testing (1000x1000+)

### Phase 3: Unity Integration (Week 4)
- [ ] Shared library build configuration
- [ ] C API wrapper
- [ ] Unity package structure
- [ ] Integration testing

### Phase 4: Polish & Documentation (Week 5)
- [ ] Comprehensive documentation
- [ ] Example configurations
- [ ] Performance benchmarks
- [ ] Unity integration guide

## Milestones

- **M1**: Console application runs basic Game of Life patterns
- **M2**: Performance benchmarks show efficient large-grid handling
- **M3**: Shared library successfully integrates with Unity project
- **M4**: Complete documentation and example implementations

## Getting Started

See [CLAUDE.md](CLAUDE.md) for development setup and [ARCHITECTURE.md](ARCHITECTURE.md) for technical details.

## Requirements

- C++20 compatible compiler
- CMake 3.20+
- EnTT (fetched automatically)
- nlohmann/json (fetched automatically)

## License

MIT License - see LICENSE file for details.