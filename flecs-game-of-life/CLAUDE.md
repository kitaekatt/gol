# FLECS Game of Life - Development Guide

## Tech Stack

- **C++20**: Modern C++ with concepts, ranges, and coroutines where applicable
- **FLECS**: High-performance Entity Component System framework
- **nlohmann/json**: JSON parsing and serialization for configuration
- **CMake 3.20+**: Build system and dependency management
- **Unity 2022.3+**: Target Unity LTS for plugin integration

## Project Structure and Naming Conventions

### Directory Structure
```
src/
├── core/              # Core ECS components and systems
├── game/              # Game of Life specific logic
├── config/            # Configuration management
├── api/               # C API layer for Unity integration
└── utils/             # Utility functions and helpers

include/
├── flecs_gol/         # Public headers matching src structure
└── flecs_gol.h        # Main include header

tests/
├── unit/              # Unit tests
├── integration/       # Integration tests
└── performance/       # Performance benchmarks

examples/
├── patterns/          # Game of Life patterns (JSON)
└── configs/           # Example configurations
```

### Naming Conventions
- **Files**: snake_case (e.g., `game_state.cpp`, `neighbor_system.h`)
- **Classes/Structs**: PascalCase (e.g., `GameState`, `NeighborSystem`)
- **Functions/Variables**: camelCase (e.g., `updateGameState`, `cellCount`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_GRID_SIZE`, `DEFAULT_TICK_RATE`)
- **Namespaces**: snake_case (e.g., `flecs_gol`, `config`)

## Build Commands

### Development Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make -j$(nproc)
```

### Release Build
```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
```

### Run Tests
```bash
cd build
ctest --output-on-failure
```

### Run Performance Benchmarks
```bash
cd build
./tests/performance/benchmark_suite
```

### Format Code
```bash
find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Static Analysis
```bash
clang-tidy src/**/*.cpp -- -Iinclude -std=c++20
```

## Performance Requirements

### Memory Efficiency
- Sparse representation: O(live_cells) memory complexity
- Maximum 64 bytes per live cell entity
- Grid size limited only by available memory
- Target: 1M+ live cells with <1GB RAM usage

### Computational Performance
- Target: 60+ FPS for 100k active cells
- Neighbor calculation: O(live_cells * 8) maximum
- Rule application: O(live_cells + births) per iteration
- Spatial queries: O(log n) lookup time for position-based queries

### Unity Integration
- Frame time budget: <1ms for ECS updates
- Memory allocation: Minimize runtime allocations
- Thread safety: Read-only access from Unity main thread
- API latency: <100μs for state queries

## Coding Style

### Code Organization
- One class per header/source file pair
- Public API in `include/`, implementation in `src/`
- Template implementations in `.inl` files when needed
- Forward declarations to minimize compile dependencies

### Error Handling
- Use exceptions for exceptional conditions only
- Return `std::optional<T>` or `std::expected<T, Error>` for fallible operations
- RAII for all resource management
- Assert for debug-only precondition checking

### Documentation
- Doxygen comments for all public APIs
- Inline comments for complex algorithms
- README files for each major subsystem
- Architecture decision records (ADRs) for significant changes

## TDD Workflow Expectations

### Test-First Development
1. Write failing test that describes desired behavior
2. Implement minimal code to make test pass
3. Refactor while maintaining test coverage
4. Commit with descriptive message

### Test Categories
- **Unit Tests**: Test individual components and systems
- **Integration Tests**: Test ECS system interactions
- **Performance Tests**: Validate performance requirements
- **API Tests**: Test C API layer functionality

### Coverage Requirements
- Minimum 80% line coverage for core functionality
- 100% coverage for public API surface
- Performance regression tests for critical paths
- Memory leak detection in long-running tests

### Continuous Integration
- All tests must pass before merge
- Performance benchmarks must not regress >5%
- Static analysis warnings treated as errors
- Code formatting automatically enforced

## Development Workflow

### Feature Development
1. Create feature branch from main
2. Write tests for new functionality
3. Implement feature with TDD approach
4. Run full test suite and benchmarks
5. Update documentation if needed
6. Create pull request with test evidence

### Debugging and Profiling
- Use FLECS debugging tools for ECS inspection
- Valgrind for memory error detection
- Perf/Intel VTune for performance profiling
- AddressSanitizer for runtime error detection

### Integration Testing
- Test with various grid sizes and patterns
- Validate Unity plugin functionality
- Cross-platform compatibility verification
- Performance testing on target hardware