#pragma once

// Main header for FLECS Game of Life library
// Include this to get access to all public APIs

#include <flecs_gol/game_config.h>
#include <flecs_gol/components.h>
#include <flecs_gol/game_of_life_simulation.h>
#include <flecs_gol/simulation_controller.h>
#include <flecs_gol/console_renderer.h>
#include <flecs_gol/console_input.h>

namespace flecs_gol {

// Library version information
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

// Utility function to get version string
inline const char* getVersionString() {
    return "1.0.0";
}

} // namespace flecs_gol