cmake_minimum_required(VERSION 3.20)
project(flecs_game_of_life VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard with modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_CONSOLE "Build console application" ON)
option(BUILD_UNITY_PLUGIN "Build Unity plugin (shared library)" OFF)
option(BUILD_EXAMPLES "Build example applications" ON)
option(ENABLE_PROFILING "Enable performance profiling" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer (Debug builds)" OFF)

# vcpkg integration
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Performance optimizations for Release builds
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
    
    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -Wpedantic")
    
    # Address Sanitizer for Debug builds
    if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Wall /DEBUG")
    
    # Enable parallel compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Find packages via vcpkg
find_package(flecs CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

if(BUILD_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)
endif()

# Include directories
include_directories(include)

# Core simulation library
add_library(flecs_gol_core STATIC
    src/config/game_config.cpp
    src/core/game_of_life_simulation.cpp
    src/core/simulation_controller.cpp
)

target_include_directories(flecs_gol_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(flecs_gol_core PUBLIC 
    flecs::flecs_static
    nlohmann_json::nlohmann_json
)

target_compile_features(flecs_gol_core PUBLIC cxx_std_20)

# Compiler-specific warnings and optimizations
target_compile_options(flecs_gol_core PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /permissive->
)

# Console application
if(BUILD_CONSOLE)
    add_executable(flecs_gol_console
        src/console/main.cpp
        src/console/console_renderer.cpp
        src/console/console_input.cpp
    )
    
    target_link_libraries(flecs_gol_console PRIVATE flecs_gol_core)
    
    # Platform-specific libraries for console input
    if(WIN32)
        # Windows console APIs
        target_link_libraries(flecs_gol_console PRIVATE user32)
    else()
        # Unix terminal APIs
        target_link_libraries(flecs_gol_console PRIVATE pthread)
    endif()
    
    # Set console as default startup project for Visual Studio
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT flecs_gol_console)
endif()

# Unity plugin (shared library)
if(BUILD_UNITY_PLUGIN)
    add_library(flecs_gol_unity SHARED
        # Unity plugin source files will be added when implementing
    )
    
    target_link_libraries(flecs_gol_unity PRIVATE flecs_gol_core)
    
    # Set proper naming for Unity plugin
    set_target_properties(flecs_gol_unity PROPERTIES
        OUTPUT_NAME "flecs_gol"
        PREFIX ""
        SUFFIX ".dll"  # Unity expects .dll on all platforms
    )
    
    # Export C API symbols
    target_compile_definitions(flecs_gol_unity PRIVATE FLECS_GOL_EXPORTS)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    add_executable(flecs_gol_tests
        tests/unit/test_game_config.cpp
        tests/unit/test_game_of_life_rules.cpp
        tests/unit/test_grid_boundaries.cpp
        tests/unit/test_flecs_entities.cpp
    )
    
    target_link_libraries(flecs_gol_tests PRIVATE 
        flecs_gol_core
        Catch2::Catch2WithMain
    )
    
    # Catch2 integration with CTest
    include(CTest)
    include(Catch)
    catch_discover_tests(flecs_gol_tests)
    
    # Performance benchmarks (separate executable)
    add_executable(flecs_gol_benchmarks
        tests/performance/benchmark_simulation.cpp
    )
    
    target_link_libraries(flecs_gol_benchmarks PRIVATE 
        flecs_gol_core
        Catch2::Catch2WithMain
    )
endif()

# Examples
if(BUILD_EXAMPLES)
    # Examples will be added as separate executables
    # add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS flecs_gol_core
    EXPORT flecs_gol_coreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets for find_package support
install(EXPORT flecs_gol_coreTargets
    FILE flecs_gol_coreTargets.cmake
    NAMESPACE flecs_gol::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flecs_gol_core
)

# Profiling support
if(ENABLE_PROFILING)
    target_compile_definitions(flecs_gol_core PUBLIC FLECS_GOL_PROFILING_ENABLED)
endif()

# Print build configuration
message(STATUS "FLECS Game of Life Build Configuration:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Console App: ${BUILD_CONSOLE}")
message(STATUS "  Unity Plugin: ${BUILD_UNITY_PLUGIN}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Profiling: ${ENABLE_PROFILING}")
message(STATUS "  AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  vcpkg: ${VCPKG_TARGET_TRIPLET}")