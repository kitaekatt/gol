# Game of Life - Flecs ECS Implementation

## Implementation Overview

This is a Flecs-based implementation of Conway's Game of Life designed as a gRPC server. The implementation leverages Flecs' advanced Entity-Component-System (ECS) framework to efficiently simulate the Game of Life using sparse representation where only live cells exist as entities.

**Key Features:**
- Sparse cell representation (only live cells exist)
- Flecs ECS world with advanced querying capabilities
- gRPC server using gRPC++ for client communication
- High-performance neighbor calculation with spatial queries
- Memory-efficient design (<1KB per 1000 live cells)
- Flecs modules for organized system architecture

## Development Environment Setup

### Prerequisites
- **C++ Compiler**: GCC 11+ or Clang 14+ with C++17 support
- **CMake**: Version 3.20 or higher
- **Protocol Buffers**: For gRPC code generation
- **gRPC**: C++ gRPC library
- **Flecs**: Modern ECS framework (included via CMake)

### Installation

#### Ubuntu/Debian
```bash
# Install build tools
sudo apt update
sudo apt install build-essential cmake pkg-config

# Install gRPC and Protocol Buffers
sudo apt install libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc

# Install additional dependencies
sudo apt install libjsoncpp-dev
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake grpc protobuf jsoncpp
```

#### Windows (MSYS2)
```bash
# Install MSYS2 dependencies
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-grpc mingw-w64-x86_64-protobuf
```

### Building the Project
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(nproc)

# Alternative: Use CMake to build
cmake --build . --config Release
```

### Dependencies
- **Flecs**: Fast and flexible ECS framework
- **gRPC++**: High-performance RPC framework
- **Protocol Buffers**: Data serialization
- **JsonCpp**: JSON parsing for patterns
- **UUID**: Unique simulation identifiers

## Architecture Summary

### ECS Components
```cpp
struct Position {
    int32_t x, y;
};

struct CellState {
    enum State { Alive, Dead, New } state;
};

struct NeighborCount {
    int32_t count;  // 0-8 neighbors
};

struct GridCell {
    // Tag component for spatial queries
};
```

### ECS Systems and Modules
Flecs implementation uses modules for organized system architecture:

```cpp
// Core simulation module
struct GameOfLifeModule {
    GameOfLifeModule(flecs::world& world);
};

// Systems organized by phase
struct NeighborSystem {
    void calculate_neighbors(flecs::iter& it);
};

struct LifecycleSystem {
    void apply_rules(flecs::iter& it);
};

struct CleanupSystem {
    void remove_dead_cells(flecs::iter& it);
};
```

### Flecs World Management
```cpp
class GameOfLifeWorld {
    flecs::world world;
    GridConfig grid_config;
    uint64_t generation;
    
    void step();
    void load_pattern(const Pattern& pattern, Position offset);
    std::vector<Cell> get_live_cells() const;
    
    // Flecs queries for efficient iteration
    flecs::query<Position, CellState> live_cells_query;
    flecs::query<Position, NeighborCount> neighbor_query;
};
```

### System Execution Pipeline
Flecs provides sophisticated system scheduling:
1. **PreFrame**: Initialize frame state
2. **OnUpdate**: 
   - Calculate neighbors using spatial queries
   - Apply Game of Life rules
   - Handle API requests
3. **PostFrame**: Cleanup dead entities

## Build Instructions

### CMake Configuration
The project uses CMake for build configuration. Key CMake options:
- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo
- `BUILD_TESTS`: Enable/disable test compilation
- `BUILD_BENCHMARKS`: Enable/disable benchmark compilation
- `FLECS_STATIC`: Link Flecs statically

### Development Build
```bash
mkdir build-debug && cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
make -j$(nproc)
```

### Release Build
```bash
mkdir build-release && cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
make -j$(nproc)
```

### Running the Server
```bash
# From build directory
./gol-flecs-server

# Custom port (default: 50051)
./gol-flecs-server --port 50052

# With logging and Flecs explorer
./gol-flecs-server --log-level info --flecs-explorer
```

### Server Configuration
Command line options:
- `--port`: gRPC server port (default: 50051)
- `--log-level`: Logging level (debug, info, warn, error)
- `--max-simulations`: Maximum concurrent simulations (default: 10)
- `--threads`: Number of worker threads (default: CPU cores)
- `--flecs-explorer`: Enable Flecs web-based explorer

## Testing Guidelines

### Running Tests
```bash
# Run all tests
cd build && ctest

# Run with verbose output
ctest -V

# Run specific test
ctest -R glider_test

# Run tests in parallel
ctest -j$(nproc)
```

### Test Categories
- **Unit Tests**: Individual component and system behavior
- **Integration Tests**: gRPC service functionality
- **Pattern Tests**: Validation of Game of Life patterns
- **Performance Tests**: Memory usage and computation benchmarks
- **Query Tests**: Flecs query performance and correctness

### Test Framework
Using Google Test (gtest) with Flecs-specific utilities:
```cpp
TEST(GameOfLifeTest, GliderPattern) {
    flecs::world world;
    GameOfLifeModule module(world);
    
    // Load glider pattern
    load_pattern_from_file(world, "glider.json", {10, 10});
    
    // Test 4 generations (glider period)
    for (int i = 0; i < 4; ++i) {
        world.progress();
    }
    
    // Verify glider movement using Flecs query
    auto live_cells = world.query<Position, CellState>();
    int count = 0;
    live_cells.each([&](Position pos, CellState state) {
        if (state.state == CellState::Alive) count++;
    });
    
    ASSERT_EQ(count, 5);
}
```

## API Documentation

### gRPC Service Interface
The server implements the `GameOfLifeService` as defined in `../proto/game_of_life.proto`.

#### Service Implementation with Flecs Integration
```cpp
class GameOfLifeServiceImpl final : public GameOfLifeService::Service {
private:
    std::unordered_map<std::string, std::unique_ptr<GameOfLifeWorld>> simulations;
    
public:
    grpc::Status GetStatus(grpc::ServerContext* context,
                          const StatusRequest* request,
                          StatusResponse* response) override;
    
    grpc::Status CreateSimulation(grpc::ServerContext* context,
                                 const CreateSimulationRequest* request,
                                 SimulationResponse* response) override;
    
    grpc::Status StepSimulation(grpc::ServerContext* context,
                               const StepSimulationRequest* request,
                               StepResponse* response) override;
};
```

#### Flecs Query Examples
```cpp
// Get all live cells
auto get_live_cells = [&]() {
    std::vector<Cell> cells;
    world.query<Position, CellState>()
        .each([&](flecs::entity e, Position pos, CellState state) {
            if (state.state == CellState::Alive) {
                cells.push_back({pos.x, pos.y, true, 0});
            }
        });
    return cells;
};

// Count neighbors using spatial query
auto count_neighbors = [&](int32_t x, int32_t y) {
    int count = 0;
    world.query<Position, CellState>()
        .each([&](Position pos, CellState state) {
            if (state.state == CellState::Alive) {
                int dx = abs(pos.x - x);
                int dy = abs(pos.y - y);
                if (dx <= 1 && dy <= 1 && !(dx == 0 && dy == 0)) {
                    count++;
                }
            }
        });
    return count;
};
```

### Pattern Loading with Flecs
```cpp
void load_pattern_from_file(flecs::world& world, 
                           const std::string& filename, 
                           Position offset) {
    std::ifstream file(filename);
    Json::Value root;
    file >> root;
    
    for (const auto& cell : root["cells"]) {
        auto entity = world.entity()
            .set<Position>({
                cell["x"].asInt() + offset.x,
                cell["y"].asInt() + offset.y
            })
            .set<CellState>({CellState::Alive})
            .add<GridCell>();
    }
}
```

## Performance Targets

### Memory Efficiency
- **Target**: <1KB per 1000 live cells
- **Sparse Storage**: Only live cells exist as entities
- **Efficient Cleanup**: Dead entities removed immediately
- **Component Pools**: Flecs memory pools for components

### Computational Performance
- **Complexity**: O(live_cells) not O(grid_size²)
- **Scalability**: Support up to 10,000x10,000 grids
- **Response Time**: gRPC calls complete within 100ms
- **Throughput**: Handle 10+ concurrent simulations

### Flecs Query Optimization
```cpp
// Cached queries for performance
class OptimizedGameOfLife {
    flecs::query<Position, CellState> live_cells;
    flecs::query<Position, NeighborCount> neighbor_calc;
    
    // Use filters for efficient iteration
    flecs::filter<Position> alive_filter;
    
    void setup_queries() {
        live_cells = world.query<Position, CellState>();
        neighbor_calc = world.query<Position, NeighborCount>();
        alive_filter = world.filter<Position>()
            .term<CellState>()
            .with(CellState::Alive);
    }
};
```

### Benchmarking
```bash
# Run performance benchmarks
cd build && ./benchmarks/gol_benchmark

# Memory usage analysis
./benchmarks/memory_benchmark

# Query performance testing
./benchmarks/query_benchmark
```

## Troubleshooting

### Common Issues

#### Build Errors
**Issue**: `Flecs not found`
**Solution**: Flecs is fetched via CMake:
```cmake
# In CMakeLists.txt
find_package(flecs REQUIRED)
target_link_libraries(gol-flecs-server flecs::flecs)
```

**Issue**: `C++17 features not supported`
**Solution**: Update compiler or specify C++17 standard:
```bash
cmake .. -DCMAKE_CXX_STANDARD=17
```

#### Runtime Errors
**Issue**: `Flecs world creation failed`
**Solution**: Check Flecs initialization:
```cpp
flecs::world world;
if (!world.is_valid()) {
    throw std::runtime_error("Failed to create Flecs world");
}
```

**Issue**: `Query performance degradation`
**Solution**: Use cached queries and filters:
```cpp
// Cache queries during initialization
auto query = world.query<Position, CellState>();
// Reuse query for multiple iterations
```

#### Performance Issues
**Issue**: High memory usage
**Solution**: Check for entity leaks:
```cpp
// Monitor entity count
std::cout << "Entity count: " << world.count<Position>() << std::endl;
```

**Issue**: Slow query execution
**Solution**: Optimize query patterns:
```cpp
// Use filters instead of queries when possible
auto filter = world.filter<Position>().term<CellState>();
```

### Debugging
Enable debug logging:
```bash
./gol-flecs-server --log-level debug
```

Use Flecs explorer for debugging:
```bash
./gol-flecs-server --flecs-explorer
# Open http://localhost:27750 in browser
```

Memory profiling:
```bash
valgrind --tool=massif ./gol-flecs-server
```

### Getting Help
- **Flecs Documentation**: https://www.flecs.dev/flecs/
- **Examples**: Check `examples/` directory for usage patterns
- **Issues**: Report bugs in project issue tracker
- **Performance**: Use Flecs explorer for system profiling

## Development Notes

### Flecs-Specific Patterns
- **World**: Central entity and component storage
- **Queries**: Efficient entity iteration and filtering
- **Systems**: Organized as functions or classes
- **Modules**: Logical grouping of systems and components
- **Observers**: Event-driven component changes

### Code Organization
```
src/
├── main.cpp             # Server entry point
├── game_of_life.h       # Main game logic interface
├── game_of_life.cpp     # Game logic implementation
├── components/          # ECS components
│   ├── position.h
│   ├── cell_state.h
│   └── neighbor.h
├── systems/             # ECS systems
│   ├── neighbor_system.h
│   ├── lifecycle_system.h
│   ├── cleanup_system.h
│   └── api_system.h
├── modules/             # Flecs modules
│   ├── game_of_life_module.h
│   └── game_of_life_module.cpp
├── grpc/               # gRPC service implementation
│   ├── service.h
│   └── service.cpp
└── utils/              # Utility functions
    ├── pattern_loader.h
    └── query_helpers.h
```

### Best Practices
- **Module Organization**: Group related systems into modules
- **Query Caching**: Cache frequently used queries
- **Component Design**: Keep components simple and focused
- **System Scheduling**: Use Flecs phases for system ordering
- **Memory Management**: Leverage Flecs automatic memory management

### CMake Integration
```cmake
# Find Flecs
find_package(flecs REQUIRED)

# Find gRPC
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)

# Create executable
add_executable(gol-flecs-server ${SOURCES})
target_link_libraries(gol-flecs-server 
    flecs::flecs
    ${GRPC_LIBRARIES}
    protobuf
    jsoncpp
)
```

### Flecs Explorer Integration
Enable the Flecs explorer for debugging and monitoring:
```cpp
#ifdef FLECS_EXPLORER
    world.import<flecs::units>();
    world.import<flecs::monitor>();
    world.set<flecs::Rest>({});
#endif
```

This implementation provides advanced ECS capabilities using Flecs' powerful query system and modular architecture while maintaining compatibility with the unified gRPC API specification.