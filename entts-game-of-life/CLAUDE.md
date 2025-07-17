# Claude Development Guide

This file contains essential information for Claude Code to effectively work on this project.

## Tech Stack

- **C++20**: Modern C++ with concepts, ranges, and coroutines
- **EnTT 3.12+**: Entity Component System library
- **nlohmann/json 3.11+**: JSON configuration persistence
- **CMake 3.20+**: Build system and dependency management
- **Google Test**: Unit testing framework (optional)
- **Google Benchmark**: Performance testing (optional)

## Project Structure

```
entts-game-of-life/
├── CMakeLists.txt              # Main build configuration
├── src/
│   ├── core/                   # Core ECS components and systems
│   ├── game/                   # Game of Life specific logic
│   ├── config/                 # Configuration management
│   ├── api/                    # C API for Unity integration
│   └── main.cpp               # Console application entry point
├── include/
│   ├── core/                   # Core headers
│   ├── game/                   # Game headers
│   ├── config/                 # Config headers
│   └── api/                    # C API headers
├── tests/                      # Unit tests
├── benchmarks/                 # Performance tests
├── examples/                   # Configuration examples
└── unity/                      # Unity integration package
```

## Naming Conventions

### Files and Directories
- **Headers**: PascalCase (`GameOfLifeSystem.h`)
- **Sources**: PascalCase (`GameOfLifeSystem.cpp`)
- **Directories**: lowercase (`core/`, `game/`)

### Code
- **Classes**: PascalCase (`GameOfLifeSystem`)
- **Functions**: camelCase (`updateGeneration()`)
- **Variables**: camelCase (`cellCount`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_GRID_SIZE`)
- **Namespaces**: snake_case (`game_of_life`)

## Build Commands

```bash
# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build --parallel

# Run tests
ctest --test-dir build --parallel

# Run benchmarks
./build/benchmarks/game_of_life_benchmark

# Generate shared library
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIB=ON
```

## Test Commands

```bash
# Run all tests
ctest --test-dir build --verbose

# Run specific test suite
./build/tests/core_tests
./build/tests/game_tests

# Run with coverage (if configured)
cmake --build build --target coverage
```

## Performance Requirements

- **Grid Size**: Support up to 10,000x10,000 cells
- **Frame Rate**: 60+ generations per second for 1000x1000 grid
- **Memory**: Under 100MB for 1000x1000 grid with 50% density
- **Startup**: Under 100ms initialization time

## Coding Style

- **C++20 features**: Use concepts, ranges, and structured bindings
- **RAII**: Always prefer stack allocation and smart pointers
- **const-correctness**: Mark everything const that can be const
- **No raw pointers**: Use references, smart pointers, or spans
- **EnTT patterns**: Follow EnTT best practices for component design

### Example Component
```cpp
struct Position {
    std::int32_t x, y;
    
    constexpr bool operator==(const Position&) const noexcept = default;
    constexpr auto operator<=>(const Position&) const noexcept = default;
};
```

## TDD Workflow

1. **Red**: Write failing test for new functionality
2. **Green**: Write minimal code to pass the test
3. **Refactor**: Clean up code while keeping tests green
4. **Performance**: Add benchmarks for critical paths

### Test Structure
- Use Google Test framework
- One test file per class/component
- Test naming: `ClassName_MethodName_ExpectedBehavior`
- Use fixtures for complex setup

### Performance Testing
- Benchmark critical systems (neighbor calculation, generation update)
- Target specific performance metrics
- Test with realistic data sizes

## Common Tasks

### Adding New Component
1. Create header in `include/core/components/`
2. Add to CMakeLists.txt if needed
3. Write unit tests
4. Update documentation

### Adding New System
1. Create header/source in `include/core/systems/` and `src/core/systems/`
2. Implement EnTT system pattern
3. Add comprehensive tests
4. Add performance benchmarks

### Configuration Changes
1. Update JSON schema
2. Modify config loading/saving
3. Update example configurations
4. Test serialization round-trip

## Debug Information

- Use CMake Debug build type for development
- Enable EnTT assertions in debug builds
- Use sanitizers for memory debugging
- Profile with target workloads (large grids)