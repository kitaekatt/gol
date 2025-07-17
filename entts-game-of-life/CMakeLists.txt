cmake_minimum_required(VERSION 3.20)
project(GameOfLife VERSION 1.0.0)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(BUILD_SHARED_LIB "Build shared library for Unity" OFF)
option(BUILD_CONSOLE_APP "Build console application" ON)

# Find packages
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Core library
add_library(game_of_life_core STATIC
    src/core/GameConfig.cpp
    src/core/GameOfLifeSimulation.cpp
)

target_include_directories(game_of_life_core PUBLIC
    include
)

target_link_libraries(game_of_life_core PUBLIC
    EnTT::EnTT
    nlohmann_json::nlohmann_json
)

# Console application
if(BUILD_CONSOLE_APP)
    add_executable(game_of_life_console
        src/console/main.cpp
        src/console/SimulationController.cpp
        src/console/ConsoleRenderer.cpp
        src/console/ConsoleInput.cpp
    )
    
    target_include_directories(game_of_life_console PRIVATE
        include
    )
    
    target_link_libraries(game_of_life_console PRIVATE
        game_of_life_core
    )
endif()

# Shared library for Unity
if(BUILD_SHARED_LIB)
    add_library(game_of_life_unity SHARED
        src/api/UnityAPI.cpp
    )
    
    target_include_directories(game_of_life_unity PRIVATE
        include
    )
    
    target_link_libraries(game_of_life_unity PRIVATE
        game_of_life_core
    )
endif()

# Tests
if(BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)
    
    # Core tests
    add_executable(core_tests
        tests/core/test_GameConfig.cpp
        tests/core/test_GameOfLifeRules.cpp
        tests/core/test_GridBoundaries.cpp
        tests/core/test_EntityLifecycle.cpp
    )
    
    target_include_directories(core_tests PRIVATE
        include
    )
    
    target_link_libraries(core_tests PRIVATE
        game_of_life_core
        Catch2::Catch2WithMain
    )
    
    # Integration tests
    add_executable(integration_tests
        tests/integration/test_HeadlessController.cpp
        tests/integration/test_ModelViewSeparation.cpp
        tests/integration/test_PatternPerformance.cpp
        tests/integration/test_Performance.cpp
        src/console/SimulationController.cpp
        src/console/ConsoleRenderer.cpp
        src/console/ConsoleInput.cpp
    )
    
    target_include_directories(integration_tests PRIVATE
        include
    )
    
    target_link_libraries(integration_tests PRIVATE
        game_of_life_core
        Catch2::Catch2WithMain
    )
    
    # Enable CTest
    enable_testing()
    add_test(NAME core_tests COMMAND core_tests)
    add_test(NAME integration_tests COMMAND integration_tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)
    
    add_executable(game_of_life_benchmark
        benchmarks/GameOfLifeBenchmarks.cpp
    )
    
    target_include_directories(game_of_life_benchmark PRIVATE
        include
    )
    
    target_link_libraries(game_of_life_benchmark PRIVATE
        game_of_life_core
        benchmark::benchmark
    )
endif()

# Simple build test
add_executable(test_simple_build test_simple_build.cpp)
target_include_directories(test_simple_build PRIVATE include)
target_link_libraries(test_simple_build PRIVATE game_of_life_core)

# Copy config files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/default.json ${CMAKE_CURRENT_BINARY_DIR}/config/default.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/glider.json ${CMAKE_CURRENT_BINARY_DIR}/config/glider.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/blinker.json ${CMAKE_CURRENT_BINARY_DIR}/config/blinker.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/gosper_gun.json ${CMAKE_CURRENT_BINARY_DIR}/config/gosper_gun.json COPYONLY)