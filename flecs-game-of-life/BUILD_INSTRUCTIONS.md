# Build Instructions

## Prerequisites

### Required Tools
- CMake 3.20 or higher
- C++20 compatible compiler (GCC 11+, Clang 13+, MSVC 2022+)
- vcpkg (for dependency management)

### vcpkg Setup

1. Install vcpkg if you haven't already:
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# or
.\bootstrap-vcpkg.bat  # Windows
```

2. Install dependencies:
```bash
./vcpkg install flecs nlohmann-json catch2
```

## Building

### Development Build with Tests
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTS=ON \
      ..
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --parallel  # Cross-platform
```

### Release Build
```bash
mkdir build-release && cd build-release
cmake -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_CONSOLE=ON \
      -DBUILD_UNITY_PLUGIN=ON \
      ..
make -j$(nproc)
```

### Running Tests
```bash
cd build
ctest --output-on-failure
# or run individual test executables
./flecs_gol_tests
./flecs_gol_benchmarks
```

## Build Options

- `BUILD_TESTS=ON/OFF` - Build unit tests and benchmarks (default: ON)
- `BUILD_CONSOLE=ON/OFF` - Build console application (default: ON)
- `BUILD_UNITY_PLUGIN=ON/OFF` - Build Unity plugin shared library (default: OFF)
- `BUILD_EXAMPLES=ON/OFF` - Build example applications (default: ON)
- `ENABLE_PROFILING=ON/OFF` - Enable performance profiling (default: OFF)
- `ENABLE_ASAN=ON/OFF` - Enable AddressSanitizer for debug builds (default: OFF)

## Troubleshooting

### vcpkg Issues
- Make sure vcpkg is properly installed and accessible
- Verify the toolchain file path is correct
- Try: `vcpkg integrate install` to set up global integration
- If vcpkg fails, you can install dependencies manually:
  - FLECS: https://github.com/SanderMertens/flecs
  - nlohmann/json: https://github.com/nlohmann/json
  - Catch2: https://github.com/catchorg/Catch2

### Dependency Issues
- FLECS: Ensure you have version 3.2.0 or higher
- nlohmann/json: Version 3.11.0 or higher required
- Catch2: Version 3.4.0 or higher for testing

### Compiler Issues
- C++20 support is required
- On older GCC versions, you might need to add: `-std=c++20`
- MSVC requires Visual Studio 2022 or later
- Clang 13+ required for full C++20 support

### Build Issues
- If CMake can't find dependencies, try setting CMAKE_PREFIX_PATH
- For threading issues on Linux, ensure pthread is linked
- Windows users may need to install Windows SDK
- If terminal input doesn't work, check console permissions

### Runtime Issues
- Make sure config/ and examples/ directories exist in working directory
- Pattern files must be valid JSON format
- Terminal must support ANSI escape codes for colors
- For best experience, use a terminal with at least 80x25 characters

### Testing Without FLECS
If you want to test basic structure compilation:
```bash
cd build
cmake -f ../CMakeLists_minimal.txt ..
make test_compilation_only
./test_compilation_only
```

## Performance Testing

The benchmark suite includes several performance tests:
```bash
./flecs_gol_benchmarks
```

Expected performance targets:
- Single step for 1000 cells: < 16ms (60 FPS target)
- Memory usage: < 1KB per 1000 live cells
- Entity creation should scale linearly

## Integration Tests

For testing known Game of Life patterns:
```bash
./flecs_gol_tests "[patterns]"
```

This will run tests for:
- Glider movement
- Blinker oscillation  
- Block still life
- Conway's rules validation