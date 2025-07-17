# Testing Guide for FLECS Game of Life

This guide outlines the testing procedure for the FLECS Game of Life implementation.

## 1. Build System Testing

### Prerequisites Check
```bash
# Verify prerequisites
cmake --version  # Should be 3.20+
gcc --version    # Should support C++20 (GCC 11+)
# or
clang --version  # Should support C++20 (Clang 13+)
```

### Basic Build Test
```bash
# Test basic structure compilation (no external deps)
mkdir build-minimal
cd build-minimal
cmake -f ../CMakeLists_minimal.txt ..
make test_compilation_only
./test_compilation_only
```

### Full Build with vcpkg
```bash
# Full build with dependencies
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTS=ON \
      -DBUILD_CONSOLE=ON \
      ..
make -j$(nproc)
```

## 2. Unit Testing

### Run All Tests
```bash
cd build
ctest --output-on-failure -V
```

### Individual Test Categories
```bash
# Configuration tests
./flecs_gol_tests "[config]"

# Game of Life rules tests  
./flecs_gol_tests "[rules]"

# Boundary handling tests
./flecs_gol_tests "[boundaries]"

# FLECS entity tests
./flecs_gol_tests "[flecs]"
```

### Expected Test Results
All tests should pass. Look for:
- GameConfig JSON serialization/deserialization
- Conway's Game of Life rule compliance
- Grid boundary enforcement
- FLECS entity lifecycle management
- Performance benchmarks within targets

## 3. Console Application Testing

### Basic Console App Test
```bash
cd build
./flecs_gol_console --help
```

### Headless Mode Testing
```bash
# Test controller without view (model/view separation)
./flecs_gol_console --headless --fps 60

# Test with pattern loading
./flecs_gol_console --headless --pattern ../examples/patterns/glider.json

# Test with custom config
./flecs_gol_console --headless --config ../config/default.json
```

### Interactive Mode Testing
```bash
# Start interactive mode
./flecs_gol_console

# Test keyboard controls:
# SPACE - pause/resume
# S     - single step
# R     - reset
# 1,2,3 - load patterns
# W,A,S,D - move view
# +,-   - zoom
# H     - help
# Q     - quit
```

## 4. Performance Testing

### Performance Benchmarks
```bash
cd build
./flecs_gol_benchmarks
```

### Performance Validation Tests
Expected performance targets:
- Single step for 1000 cells: < 16ms (60 FPS)
- Memory usage: < 1KB per 1000 live cells
- Entity creation scales linearly

### Large Grid Testing
Test with different configurations:
```bash
# Small grid (performance test config)
./flecs_gol_console --config ../examples/configs/performance_test.json

# Custom large grid
./flecs_gol_console --headless --fps 30  # Lower FPS for large grids
```

## 5. Pattern Testing

### Test All Patterns
```bash
# Test each pattern individually
./flecs_gol_console --pattern ../examples/patterns/glider.json
./flecs_gol_console --pattern ../examples/patterns/blinker.json  
./flecs_gol_console --pattern ../config/patterns/gosper_gun.json
```

### Pattern Validation
Expected behaviors:
- **Glider**: Should move diagonally, repeat every 4 generations
- **Blinker**: Should oscillate between horizontal/vertical, period 2
- **Gosper Gun**: Should produce gliders every 30 generations

## 6. Configuration Testing

### Config Persistence
```bash
# Test config loading
./flecs_gol_console --config ../config/default.json

# Test config validation
./flecs_gol_console --config non_existent_file.json  # Should use defaults
```

### Config File Validation
Test with modified config files:
- Invalid JSON syntax
- Missing required fields
- Invalid value ranges (negative FPS, etc.)

## 7. User Experience Testing

### Terminal Compatibility
Test in different terminals:
- Standard terminal (bash, zsh)
- Windows Command Prompt
- Windows PowerShell
- Terminal with limited color support

### Edge Cases
```bash
# Empty grid
./flecs_gol_console  # Start with no pattern, test all controls

# Very dense patterns
# Create a pattern with many cells and test performance
```

### Clean Exit Testing
- Test Ctrl+C signal handling
- Test Q key exit
- Test ESC key exit
- Verify terminal state is restored

## 8. Model/View Separation Validation

### Headless Operation
```bash
# Run headless test to verify controller independence
./test_headless
```

### Controller Interface Testing
Verify the controller can:
- Run without any view components
- Provide state information on demand
- Handle patterns and configuration independently
- Maintain thread-safe operations

## 9. Cross-Platform Testing

### Windows-Specific Tests
- Console input handling with Windows console API
- ANSI color code support
- Path handling (backslashes vs forward slashes)

### Unix-Specific Tests
- Terminal control (termios)
- Signal handling (SIGINT, SIGTERM)
- Path handling and file permissions

## 10. Integration Testing

### Full Workflow Test
1. Build project successfully
2. Run unit tests (all pass)
3. Load default configuration
4. Load and run different patterns
5. Test interactive controls
6. Verify performance targets
7. Test clean shutdown

### Stress Testing
- Run for extended periods (30+ minutes)
- Test with maximum entity counts
- Test rapid pattern switching
- Test zoom/pan operations extensively

## Expected Outcomes

### Success Criteria
- All unit tests pass
- Console application starts and responds to input
- Patterns load and evolve correctly according to Conway's rules
- Performance targets are met
- Application exits cleanly
- Model/view separation is maintained

### Common Issues and Solutions
- **Build failures**: Check C++20 support and dependencies
- **Test failures**: Verify FLECS and nlohmann/json versions
- **Console input issues**: Check terminal capabilities and permissions
- **Pattern loading errors**: Verify JSON syntax and file paths
- **Performance issues**: Check compiler optimizations and build type

## Reporting Issues
Document any issues found with:
- Operating system and version
- Compiler and version
- Build configuration used
- Specific test that failed
- Expected vs actual behavior
- Console output/error messages