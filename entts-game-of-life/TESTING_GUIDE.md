# Testing Guide

## Quick Test Commands

### 1. Build and Basic Test
```bash
# Configure and build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON
cmake --build build

# Run all tests
ctest --test-dir build --verbose

# Run console app
./build/game_of_life_console
```

## Comprehensive Testing Checklist

### Unit Tests Validation
- [ ] **Config Tests**: `./build/core_tests` - GameConfig serialization/validation
- [ ] **Rules Tests**: Conway's Game of Life rules implementation  
- [ ] **Boundary Tests**: Grid edge handling (wrapping/non-wrapping)
- [ ] **Entity Tests**: EnTT entity lifecycle management
- [ ] **Performance Tests**: `./build/integration_tests` - Benchmark validation

### Console Application Testing

#### Pattern Loading Tests
```bash
# Test pattern files exist and load correctly
ls config/
# Should show: default.json, glider.json, blinker.json, gosper_gun.json
```

#### Keyboard Controls Test
Launch console app and test each control:
- [ ] **SPACE**: Start/Pause simulation
- [ ] **S**: Single step (when paused)
- [ ] **R**: Reset simulation 
- [ ] **Q**: Quit application
- [ ] **W/A/S/D**: Move viewport (Pan view)
- [ ] **L**: Load pattern menu
- [ ] **I**: Toggle stats display
- [ ] **H**: Toggle controls display

#### Visual Verification
- [ ] **Grid Rendering**: Cells display as `█` (alive) and ` ` (dead)
- [ ] **Stats Display**: Shows Generation, Cells, FPS, Step time
- [ ] **Controls Display**: Shows keyboard shortcuts
- [ ] **Viewport Panning**: View moves smoothly with WASD

### Performance Testing

#### Small Grid Performance (100x100)
```bash
# Run with glider pattern - should maintain 60+ FPS
# Monitor: Generation count increases smoothly
# Verify: Memory usage stays constant
```

#### Medium Grid Performance (500x500) 
```bash
# Test with random pattern (~5% density)
# Target: 30+ FPS
# Check: No memory leaks over time
```

#### Large Grid Performance (1000x1000+)
```bash
# Sparse pattern test
# Verify: Performance scales with living cells, not grid size
# Monitor: Memory usage under 100MB
```

### Pattern-Specific Tests

#### Glider Pattern
- [ ] **Movement**: Travels diagonally (1 cell per 4 generations)
- [ ] **Stability**: Pattern maintains shape while moving
- [ ] **Boundaries**: Behaves correctly at grid edges

#### Blinker Pattern  
- [ ] **Oscillation**: Alternates horizontal/vertical every generation
- [ ] **Period**: Returns to original state after 2 generations
- [ ] **Stability**: Continues oscillating indefinitely

#### Gosper Gun Pattern
- [ ] **Gun Operation**: Shoots gliders every 30 generations
- [ ] **Growth**: Population increases over time
- [ ] **Complexity**: Handles complex multi-part pattern

### Configuration Testing

#### Grid Size Validation
```bash
# Test various grid sizes
# Small: 20x20, Medium: 100x100, Large: 1000x1000
# Verify: All sizes work without crashes
```

#### Wrapping Mode Test
```bash
# Test both wrapping and non-wrapping edge behavior
# Place pattern near edge, verify boundary handling
```

#### Performance Settings
```bash
# Test different FPS targets: 10, 30, 60, 120
# Verify: Actual FPS matches target (within tolerance)
```

### Edge Cases and Stress Testing

#### Empty Grid Test
- [ ] **Start Empty**: Launch with no living cells
- [ ] **All Die**: Patterns that die out completely
- [ ] **Performance**: Empty grid operations are fast

#### Dense Pattern Test  
- [ ] **High Density**: Fill 50%+ of grid with living cells
- [ ] **Performance**: System handles dense patterns
- [ ] **Memory**: No excessive memory usage

#### Boundary Stress Test
- [ ] **Corner Patterns**: Place patterns at grid corners
- [ ] **Edge Wrapping**: Test wrapping at all edges
- [ ] **Large Coordinates**: Test near maximum grid size

### Model/View Separation Validation

#### Headless Operation Test
```bash
# Create simple test program or modify main:
# Run SimulationController without any view components
# Verify: Simulation runs without console renderer
```

#### Multiple View Test
```bash
# Verify: Same controller can support different render configurations
# Test: Multiple ConsoleRenderer instances on same controller
```

#### Performance Isolation
```bash
# Compare: Headless vs. with-rendering performance
# Verify: View complexity doesn't affect simulation speed
```

## Expected Results

### Performance Targets
- **100x100 Grid**: 60+ FPS with moderate patterns
- **500x500 Grid**: 30+ FPS with 5% density
- **1000x1000 Grid**: 10+ FPS with sparse patterns (<1% density)
- **Memory Usage**: Under 100MB for grids up to 1000x1000

### Stability Requirements
- **No Crashes**: All operations should be safe
- **Clean Exit**: Application exits without errors
- **Memory Stable**: No memory leaks over extended runs
- **Deterministic**: Same initial state produces same results

## Issue Reporting Template

If you find issues, please report using this format:

```
**Issue**: Brief description
**Platform**: Windows/Linux/macOS + compiler version
**Steps to Reproduce**: 
1. 
2. 
3.

**Expected Behavior**: 
**Actual Behavior**: 
**Performance Impact**: FPS/Memory measurements if relevant
**Console Output**: Any error messages
**Workaround**: If you found one
```

## Debug Tips

### Enable Debug Logging
```cpp
// Add to main.cpp for debugging:
controller.setStepCallback([](const SimulationStats& stats) {
    std::cout << "Gen: " << stats.generation 
              << " Cells: " << stats.livingCells << std::endl;
});
```

### Memory Debugging (Linux/macOS)
```bash
# Build with debug mode (includes AddressSanitizer)
cmake -B build -DCMAKE_BUILD_TYPE=Debug [other options]
cmake --build build
./build/game_of_life_console
```

### Performance Profiling
```bash
# Linux perf
perf record ./build/game_of_life_console
perf report

# Or use built-in timing
# Stats display shows step time in milliseconds
```