# Game of Life Patterns

This directory contains standard Game of Life patterns for testing and validation across all implementations (Bevy, EnTT, Flecs). Each pattern is defined in JSON format with validation criteria.

## Pattern Format

```json
{
  "name": "Pattern Name",
  "description": "Brief description of the pattern",
  "author": "Original creator",
  "pattern_type": "still_life|oscillator|spaceship|methuselah|gun",
  "period": number,
  "cells": [
    {"x": 0, "y": 0},
    {"x": 1, "y": 0}
  ],
  "validation": {
    "expected_behavior": "What should happen when this pattern runs",
    "test_steps": number,
    "stable_at_step": number,
    "final_live_cells": number
  }
}
```

## Pattern Types

### Still Life
Patterns that never change once established.
- **Period**: Always 1
- **Behavior**: Remains identical across all generations
- **Examples**: Block, Beehive, Loaf, Boat

### Oscillator
Patterns that return to their initial state after a fixed number of generations.
- **Period**: Number of generations to return to initial state
- **Behavior**: Cycles through states and returns to original
- **Examples**: Blinker (period 2), Toad (period 2), Beacon (period 2)

### Spaceship
Patterns that translate themselves across the grid.
- **Period**: Number of generations to return to initial state at new position
- **Behavior**: Moves across the grid while maintaining shape
- **Examples**: Glider (period 4), Lightweight spaceship (period 4)

### Methuselah
Patterns that evolve for a long time before stabilizing.
- **Period**: Final stable period (or death)
- **Behavior**: Chaotic evolution before reaching stable state
- **Examples**: Acorn (stabilizes after 5206 generations), Diehard (dies after 130 generations)

### Gun
Patterns that periodically emit other patterns (usually spaceships).
- **Period**: Time between emissions
- **Behavior**: Continuously creates new patterns
- **Examples**: Gosper glider gun (period 30)

## Validation Criteria

Each pattern must be validated against Conway's Game of Life rules:

1. **Underpopulation**: Live cell with < 2 live neighbors dies
2. **Survival**: Live cell with 2-3 live neighbors survives
3. **Overpopulation**: Live cell with > 3 live neighbors dies
4. **Reproduction**: Dead cell with exactly 3 live neighbors becomes alive

## Test Requirements

All implementations must correctly handle:

### Standard Test Suite
- **Block**: Must remain stable (still life)
- **Blinker**: Must oscillate between horizontal and vertical (period 2)
- **Glider**: Must move diagonally southeast (period 4)
- **Toad**: Must oscillate between two states (period 2)
- **Beacon**: Must oscillate between two states (period 2)

### Extended Test Suite
- **Glider Gun**: Must continuously emit gliders (period 30)
- **Acorn**: Must stabilize after exactly 5206 generations
- **Diehard**: Must die completely after exactly 130 generations

## Testing Protocol

1. **Load Pattern**: Initialize simulation with pattern
2. **Run Simulation**: Execute for specified number of steps
3. **Validate State**: Check against expected final state
4. **Validate Behavior**: Confirm pattern exhibited expected behavior
5. **Performance Check**: Verify memory and time requirements met

## Implementation Notes

- Coordinates are relative to pattern origin (0,0)
- All patterns assume infinite grid (implementations may vary)
- Patterns should work identically across all ECS implementations
- Performance requirements: < 1KB memory per 1000 live cells
- Response time requirements: < 100ms per generation step