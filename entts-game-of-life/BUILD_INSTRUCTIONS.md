# Build Instructions

## Prerequisites

### Required Tools
- **CMake 3.20+** - Build system
- **C++20 Compatible Compiler**:
  - GCC 10+ or Clang 10+ (Linux)
  - Visual Studio 2019+ or clang (Windows)
  - Xcode 12+ or clang (macOS)
- **vcpkg** - Package manager for dependencies

## Quick Start (Step by Step)

### Step 1: Install vcpkg
```bash
# Install vcpkg (from any directory, e.g., your home directory)
cd ~
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh   # Linux/macOS

# Note the full path - you'll need it
pwd
# Example: /home/username/vcpkg
```

### Step 2: Install Dependencies
```bash
# From vcpkg directory
./vcpkg install entt nlohmann-json catch2
```

### Step 3: Configure Project
```bash
# Go to project directory
cd /mnt/c/Dev/entts-game-of-life

# Configure (replace with your actual vcpkg path)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=/home/username/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_TESTS=ON
```

### Step 4: Build Core Components
```bash
# Build everything
cmake --build build --parallel

# Or build step by step:
# 1. First test core functionality
# ./build/test_simple_build

# 2. Then run tests
# ctest --test-dir build --verbose

# 3. Finally run console app
# ./build/game_of_life_console
```

## Alternative: Build without vcpkg (System packages)

If vcpkg gives issues, try system package manager:

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libentt-dev nlohmann-json3-dev catch2

# Configure without vcpkg
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake git entt nlohmann-json catch2-v3
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build
```

## Testing

### Run Unit Tests
```bash
# All tests
ctest --test-dir build --parallel --verbose

# Specific test suites
./build/core_tests
./build/integration_tests

# Windows:
# build\Debug\core_tests.exe
# build\Debug\integration_tests.exe
```

### Run Console Application
```bash
# Basic execution
./build/game_of_life_console

# Windows:
# build\Debug\game_of_life_console.exe
```

## Build Configurations

### Debug Build (Development)
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_TESTS=ON

cmake --build build
```

### Release Build (Performance)
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_TESTS=OFF

cmake --build build
```

### Unity Shared Library
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_SHARED_LIB=ON

cmake --build build
```

### With Benchmarks
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_BENCHMARKS=ON

# Install Google Benchmark via vcpkg first:
# vcpkg install benchmark

cmake --build build
./build/game_of_life_benchmark
```

## Troubleshooting

### Common Issues

1. **"cmake: command not found"**
   - Install CMake from https://cmake.org/download/
   - Add to PATH

2. **"Could not find vcpkg toolchain"**
   - Verify VCPKG_ROOT path
   - Ensure vcpkg is bootstrapped

3. **"Package 'entt' not found"**
   ```bash
   vcpkg install entt nlohmann-json catch2
   ```

4. **Compiler errors with C++20**
   - Ensure compiler supports C++20
   - Update to newer compiler version

5. **Missing dependencies on Linux**
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake git

   # CentOS/RHEL
   sudo yum groupinstall "Development Tools"
   sudo yum install cmake git
   ```

### Build Cache Issues
```bash
# Clean and rebuild
rm -rf build/
cmake -B build [options]
cmake --build build
```

### vcpkg Cache Issues
```bash
# Clear vcpkg cache
vcpkg remove entt nlohmann-json catch2
vcpkg install entt nlohmann-json catch2
```

## Performance Notes

- **Debug builds** include address sanitizer on non-Windows platforms
- **Release builds** use `-O3` optimization and `-march=native`
- **Large grids** (1000x1000+) may require Release build for acceptable performance
- **Memory usage** scales with living cell count, not grid size