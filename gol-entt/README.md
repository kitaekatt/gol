# Game of Life - EnTT ECS Implementation

## Implementation Overview

This is an EnTT-based implementation of Conway's Game of Life designed as a gRPC server. The implementation leverages EnTT's high-performance Entity-Component-System (ECS) library to efficiently simulate the Game of Life using sparse representation where only live cells exist as entities.

**Key Features:**
- Sparse cell representation (only live cells exist)
- EnTT ECS registry for entity management
- gRPC server using gRPC++ for client communication
- High-performance neighbor calculation with spatial hashing
- Memory-efficient design (<1KB per 1000 live cells)

## Development Environment Setup

### Prerequisites
- **C++ Compiler**: GCC 11+ or Clang 14+ with C++20 support
- **CMake**: Version 3.20 or higher
- **Protocol Buffers**: For gRPC code generation
- **gRPC**: C++ gRPC library
- **EnTT**: Header-only ECS library (included via CMake)

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
- **EnTT**: Fast and reliable ECS library
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
```

### ECS Systems
- **NeighborSystem**: Calculates neighbor counts using spatial hashing
- **LifecycleSystem**: Applies Game of Life rules to determine cell fate
- **ApiSystem**: Handles gRPC client requests and responses
- **CleanupSystem**: Removes dead cells to maintain sparse representation

### Registry Management
```cpp
class GameOfLifeWorld {
    entt::registry registry;
    GridConfig grid_config;
    uint64_t generation;
    
    void step();
    void load_pattern(const Pattern& pattern, Position offset);
    std::vector<Cell> get_live_cells() const;
};
```

### System Execution Order
1. **NeighborSystem** - Calculate neighbors using spatial hash
2. **LifecycleSystem** - Apply Game of Life rules
3. **CleanupSystem** - Remove dead entities
4. **ApiSystem** - Process client requests

## Build Instructions

### CMake Configuration
The project uses CMake for build configuration. Key CMake options:
- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo
- `BUILD_TESTS`: Enable/disable test compilation
- `BUILD_BENCHMARKS`: Enable/disable benchmark compilation

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
./gol-entt-server

# Custom port (default: 50051)
./gol-entt-server --port 50052

# With logging
./gol-entt-server --log-level info
```

### Server Configuration
Command line options:
- `--port`: gRPC server port (default: 50051)
- `--log-level`: Logging level (debug, info, warn, error)
- `--max-simulations`: Maximum concurrent simulations (default: 10)
- `--threads`: Number of worker threads (default: CPU cores)

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

### Test Framework
Using Google Test (gtest) for unit testing:
```cpp
TEST(GameOfLifeTest, GliderPattern) {
    GameOfLifeWorld world(100, 100);
    world.load_pattern(load_pattern_from_file("glider.json"), {10, 10});
    
    // Test 4 generations (glider period)
    for (int i = 0; i < 4; ++i) {
        world.step();
    }
    
    // Glider should have moved diagonally
    auto cells = world.get_live_cells();
    ASSERT_EQ(cells.size(), 5);
    // Additional validation logic...
}
```

## API Documentation

### gRPC Service Interface
The server implements the `GameOfLifeService` as defined in `../proto/game_of_life.proto`.

#### Service Implementation
```cpp
class GameOfLifeServiceImpl final : public GameOfLifeService::Service {
    grpc::Status GetStatus(grpc::ServerContext* context,
                          const StatusRequest* request,
                          StatusResponse* response) override;
    
    grpc::Status CreateSimulation(grpc::ServerContext* context,
                                 const CreateSimulationRequest* request,
                                 SimulationResponse* response) override;
    
    grpc::Status StepSimulation(grpc::ServerContext* context,
                               const StepSimulationRequest* request,
                               StepResponse* response) override;
    // ... other methods
};
```

#### Client Connection Example
```cpp
// Create channel
auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
auto stub = GameOfLifeService::NewStub(channel);

// Create simulation
CreateSimulationRequest request;
request.set_width(100);
request.set_height(100);
request.set_initial_pattern("glider");

SimulationResponse response;
grpc::ClientContext context;
grpc::Status status = stub->CreateSimulation(&context, request, &response);
```

### Pattern Loading
Patterns are loaded from JSON files in the `../patterns/` directory:
```cpp
Pattern load_pattern_from_file(const std::string& filename) {
    std::ifstream file(filename);
    Json::Value root;
    file >> root;
    
    Pattern pattern;
    pattern.name = root["name"].asString();
    pattern.description = root["description"].asString();
    
    for (const auto& cell : root["cells"]) {
        Position pos{cell["x"].asInt(), cell["y"].asInt()};
        pattern.cells.push_back(pos);
    }
    
    return pattern;
}
```

## Performance Targets

### Memory Efficiency
- **Target**: <1KB per 1000 live cells
- **Sparse Storage**: Only live cells exist as entities
- **Efficient Cleanup**: Dead entities removed immediately
- **Memory Pool**: Custom allocators for entity management

### Computational Performance
- **Complexity**: O(live_cells) not O(grid_size²)
- **Scalability**: Support up to 10,000x10,000 grids
- **Response Time**: gRPC calls complete within 100ms
- **Throughput**: Handle 10+ concurrent simulations

### Spatial Hashing
EnTT implementation uses spatial hashing for efficient neighbor lookup:
```cpp
class SpatialHashNeighborSystem {
    std::unordered_map<uint64_t, std::vector<entt::entity>> spatial_hash;
    
    uint64_t hash_position(int32_t x, int32_t y) const {
        return static_cast<uint64_t>(x) | (static_cast<uint64_t>(y) << 32);
    }
    
    int count_neighbors(int32_t x, int32_t y) const;
};
```

### Benchmarking
```bash
# Run performance benchmarks
cd build && ./benchmarks/gol_benchmark

# Memory usage analysis
./benchmarks/memory_benchmark

# Throughput testing
./benchmarks/concurrent_simulations
```

## Troubleshooting

### Common Issues

#### Build Errors
**Issue**: `EnTT not found`
**Solution**: EnTT is header-only and fetched via CMake:
```cmake
# In CMakeLists.txt
find_package(EnTT REQUIRED)
target_link_libraries(gol-entt-server EnTT::EnTT)
```

**Issue**: `gRPC libraries not found`
**Solution**: Install gRPC development packages:
```bash
sudo apt install libgrpc++-dev libprotobuf-dev
```

**Issue**: `C++20 features not supported`
**Solution**: Update compiler or specify C++20 standard:
```bash
cmake .. -DCMAKE_CXX_STANDARD=20
```

#### Runtime Errors
**Issue**: `gRPC server failed to start`
**Solution**: Check if port is available:
```bash
netstat -tlnp | grep 50051
```

**Issue**: `Pattern file not found`
**Solution**: Verify pattern files exist in `../patterns/` directory:
```bash
ls -la ../patterns/
```

#### Performance Issues
**Issue**: High memory usage
**Solution**: Check for memory leaks:
```bash
valgrind --leak-check=full ./gol-entt-server
```

**Issue**: Slow simulation steps
**Solution**: Enable compiler optimizations:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
```

### Debugging
Enable debug logging:
```bash
./gol-entt-server --log-level debug
```

Use GDB for debugging:
```bash
gdb ./gol-entt-server
(gdb) run --port 50051
```

Memory profiling with Valgrind:
```bash
valgrind --tool=massif ./gol-entt-server
```

### Getting Help
- **Documentation**: Check `docs/` directory for detailed API docs
- **Examples**: Review `examples/` directory for usage patterns
- **Issues**: Report bugs in project issue tracker
- **Performance**: Use `perf` for profiling

## Development Notes

### EnTT-Specific Patterns
- **Registry**: Central entity storage and management
- **Views**: Efficient entity iteration and filtering
- **Groups**: Optimized access patterns for related components
- **Signals**: Event-driven component changes

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
├── grpc/               # gRPC service implementation
│   ├── service.h
│   └── service.cpp
└── utils/              # Utility functions
    ├── pattern_loader.h
    └── spatial_hash.h
```

### Best Practices
- **RAII**: Use smart pointers and automatic resource management
- **Const Correctness**: Maintain const correctness throughout
- **Exception Safety**: Implement proper exception handling
- **Performance**: Profile regularly and optimize hot paths
- **Memory Management**: Use custom allocators for entity pools

### CMake Integration
```cmake
# Find EnTT
find_package(EnTT REQUIRED)

# Find gRPC
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)

# Create executable
add_executable(gol-entt-server ${SOURCES})
target_link_libraries(gol-entt-server 
    EnTT::EnTT
    ${GRPC_LIBRARIES}
    protobuf
    jsoncpp
)
```

This implementation provides high-performance Game of Life simulation using EnTT's efficient ECS architecture while maintaining compatibility with the unified gRPC API specification.