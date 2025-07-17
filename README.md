# Game of Life Multi-Implementation Project

A documentation-driven project implementing Conway's Game of Life using three different ECS architectures, with a unified client-server design ensuring functional equivalence across all implementations.

## Project Overview

This project demonstrates how to build functionally identical Game of Life simulations using three different technology stacks:

- **gol-bevy**: Rust implementation using Bevy ECS
- **gol-entt**: C++ implementation using EnTT ECS  
- **gol-flecs**: C++ implementation using Flecs ECS
- **gol-console-client**: Unified terminal client for testing all implementations

## Architecture Philosophy

### Server-Only Implementations
Each Game of Life implementation is designed as a standalone server that:
- Exposes a standardized API for simulation control
- Maintains its own simulation state
- Provides consistent data formats across all implementations
- Operates independently without built-in visualization

### Unified Console Client
A single console client (`gol-console-client`) provides:
- Interactive terminal interface for all three implementations
- Consistent user experience across different backends
- Standardized testing and validation workflows
- Performance comparison capabilities

## Project Structure

```
/mnt/c/Dev/gol/
├── README.md                    # This file
├── ARCHITECTURE.md              # Unified design principles
├── REQUIREMENTS.md              # Functional specifications
├── PROJECTPLAN.md              # Development phases and tasks
├── CLAUDE.md                   # Orchestrator instructions
├── gol-bevy/                   # Bevy ECS implementation
├── gol-entt/                   # EnTT ECS implementation
├── gol-flecs/                  # Flecs ECS implementation
├── gol-console-client/         # Unified terminal client
├── bevy-game-of-life/          # Legacy implementation (archived)
├── entts-game-of-life/         # Legacy implementation (archived)
└── flecs-game-of-life/         # Legacy implementation (archived)
```

## Development Approach

### Documentation-Driven Development
- **Requirements First**: All functionality is specified in REQUIREMENTS.md before implementation
- **Task-Based**: Each implementation directory contains a `tasks/` folder with specific .md files
- **Agent-Based**: Separate Claude Code agents work on individual tasks
- **Orchestrated**: Main orchestrator manages task creation and project coordination

### Quality Assurance
- **Functional Equivalence**: All implementations must produce identical results
- **Performance Testing**: Standardized benchmarks across all implementations
- **Automated Validation**: Consistent testing through unified client
- **Continuous Integration**: Automated builds and tests for all implementations

## Getting Started

### Prerequisites
- Rust (for Bevy implementation)
- C++ compiler with C++17 support (for EnTT and Flecs)
- CMake 3.16+ (for C++ implementations)
- No additional package installations required (all dependencies managed through project configuration)

### Quick Start
1. Build desired implementation (see individual project directories)
2. Build console client: `cd gol-console-client && [build command]`
3. Run with: `./gol-console-client --backend [bevy|entt|flecs]`

## Testing and Validation

### Functional Testing
All implementations are validated against:
- Standard Game of Life patterns (glider, blinker, etc.)
- Edge cases and boundary conditions
- Performance benchmarks with large grids
- Memory usage constraints

### Interoperability Testing
The console client ensures:
- Consistent API responses across backends
- Identical simulation results
- Uniform performance characteristics
- Standardized error handling

## Contributing

### For Implementation Teams
1. Check your implementation's `tasks/` directory for pending work
2. Complete tasks according to specifications in REQUIREMENTS.md
3. Follow architecture guidelines in ARCHITECTURE.md
4. Delete completed task files and report outcomes to orchestrator

### For Orchestrator
1. Create task files in implementation directories
2. Monitor progress and update PROJECTPLAN.md
3. Coordinate testing and validation phases
4. Ensure functional equivalence across implementations

## Project Status

Current phase: **Documentation and Planning**

See PROJECTPLAN.md for detailed status and upcoming milestones.

## Legacy Implementations

The `bevy-game-of-life/`, `entts-game-of-life/`, and `flecs-game-of-life/` directories contain previous implementations that serve as historical artifacts and inspiration for the new development effort. These are maintained for reference but not actively developed.

## Documentation

- [ARCHITECTURE.md](./ARCHITECTURE.md) - Unified design principles and technical architecture
- [REQUIREMENTS.md](./REQUIREMENTS.md) - Functional specifications and acceptance criteria
- [PROJECTPLAN.md](./PROJECTPLAN.md) - Development phases, tasks, and milestones
- [CLAUDE.md](./CLAUDE.md) - Instructions for Claude Code orchestrator and agents