# CLAUDE.md - Game of Life Multi-Implementation Project

This file provides comprehensive guidance for Claude Code when working with the Game of Life multi-implementation project. This project uses a mode-based system where Claude Code operates in different specialized modes with specific responsibilities and permissions.

## Project Overview

This repository contains a documentation-driven project to build three functionally equivalent Game of Life implementations using different ECS architectures (Bevy, EnTT, Flecs) with a unified console client.

## Current Project Structure
/mnt/c/dev/gol/
├── README.md                    # Project overview
├── ARCHITECTURE.md              # Unified design principles
├── REQUIREMENTS.md              # Functional specifications
├── PROJECTPLAN.md              # Development phases and tasks
├── CLAUDE.md                   # This file
├── gol-bevy/                   # Bevy ECS implementation
├── gol-entt/                   # EnTT ECS implementation
├── gol-flecs/                  # Flecs ECS implementation
├── gol-console-client/         # Unified terminal client
├── bevy-game-of-life/          # Legacy implementation (archived)
├── entts-game-of-life/         # Legacy implementation (archived)
└── flecs-game-of-life/         # Legacy implementation (archived)

## Mode Identification
**IMPORTANT**: Claude MUST end every response with `[Mode:{ModeName}]` to clearly indicate the current operating mode.

## Quick Reference

### Current Mode: Meta Mode
**META MODE ACTIVATED**

I am now operating as a meta-development agent with the following active constraints:
- Work exclusively on tasks in meta/tasks/
- Implement memory, learning, and workflow improvements
- Optimize documentation and development processes
- Full read/write access to meta/ directory
- Write access to any directory with careful coordination
- Self-check before modifying files in other modes' directories
- Escalate to orchestrator when cross-mode coordination needed

### Available Commands
- **"commands"** - Display complete command reference
- **"Continue development"** - Work on next highest priority task immediately
- **"What's next"** - Summarize next task without executing it
- **"Switch to [mode] mode"** - Change to specified mode
- **"Update the orchestrator"** - Provide structured completion report (stay in current mode)
- **"Orchestrate [task description]"** - Create task file and spawn sub-task using Task tool (orchestrator mode only)
- **"Begin shutdown"** - Initiate proper shutdown sequence with quality gates
- **"Is documentation up to date?"** - Verify documentation currency before shutdown
- **"Session status"** - Show current task status and shutdown readiness

### Available Modes
- **Default Mode**: Entry point and mode selection
- **Orchestrator Mode**: Project coordination and task management
- **Bevy Mode**: Bevy ECS implementation development
- **EnTT Mode**: EnTT ECS implementation development
- **Flecs Mode**: Flecs ECS implementation development
- **Console Mode**: Console client development using gRPC
- **Meta Mode**: Wholistic project improvements and meta-development tasks

## Core Documentation Imports

### User Commands and Mode System
@meta/docs/commands.md
@meta/docs/modes.md

### Task Management and Development
@meta/docs/tasks.md
@meta/docs/development.md

### Architecture and Operations
@meta/docs/architecture.md
@meta/docs/operations.md

### Documentation-First Process
@meta/docs/documentation-first-process.md
@meta/docs/shutdown-process.md

## Essential Workflow Notes

### Command Recognition Priority
1. **Immediate Recognition**: Match user input against command patterns first, before any tool usage
2. **No Tool Usage**: Commands should be executed directly without bash, filesystem, or search tools
3. **Contextual Response**: Provide response based on current mode and available tasks
4. **Orchestration Recognition**: When user says "orchestrate [task]", create task file and spawn sub-task

### Mode Switch Protocol
**AUTOMATIC MODE SWITCHES** (no verification needed):
- "as [MODENAME]", "switch to [MODENAME] mode", "become [MODENAME]"
- "work on [implementation] implementation", "work on console client", "work on meta improvements"
- "coordinate project", "return to default mode"

**CRITICAL DISTINCTION**:
- **"Update the orchestrator"** = STAY IN CURRENT MODE and provide a report
- **"Switch to orchestrator mode"** = CHANGE TO ORCHESTRATOR MODE

### Context Separation Protocol
Use `/clear` commands at strategic points:
- Between major mode changes
- Before entering specialized modes
- After complex mode sessions
- When starting new major tasks
- After error recovery

## Important Reminders

1. **Documentation First**: Always refer to REQUIREMENTS.md and ARCHITECTURE.md before starting work
2. **Command Documentation**: When creating new commands, IMMEDIATELY document them in CLAUDE.md
3. **Development Continuation**: When asked to "continue development", immediately work on the next highest priority task
4. **Command Recognition**: When user says "commands", display the complete command reference from imported docs
5. **Memory Maintenance**: Execute maintenance protocols regularly to prevent documentation drift
6. **Documentation Validation**: Verify documentation matches current project reality during reviews
7. **Orchestration Workflow**: When user says "orchestrate [task]", create task file and spawn sub-task with full communication flow visibility

## Workflow Optimization Notes

- **Tool Efficiency Strategies**: 
  * When reflecting on performance, consider cases where multiple tool calls were required to accomplish a task, and it is likely this pattern will repeat.
  * Consider solutions like making dedicated bash scripts that can quickly and with a single tool call give you the information you need.
  * When executing work, consider if a bash script will allow you to accomplish work more quickly.
  * These scripts must be in a tmp directory of some sort and be deleted as they are task-specific.

## Task Analysis Script - CRITICAL WORKFLOW IMPROVEMENT

**IMPORTANT**: All agents MUST use the task analysis script for efficient task discovery:

### Usage for Each Agent Mode:
- **Bevy**: `Bash(command="bash meta/scripts/analyze-agent-tasks.sh bevy")`
- **EnTT**: `Bash(command="bash meta/scripts/analyze-agent-tasks.sh entt")`  
- **Flecs**: `Bash(command="bash meta/scripts/analyze-agent-tasks.sh flecs")`
- **Console**: `Bash(command="bash meta/scripts/analyze-agent-tasks.sh console")`
- **Meta**: `Bash(command="bash meta/scripts/analyze-agent-tasks.sh meta")`

### Performance Improvement:
- **Before**: 5-10 tool calls (Glob + multiple Read calls + coordination checks)
- **After**: 1 tool call (single bash script execution)
- **Result**: Complete task analysis with priority recommendations and coordination status

### Script Output:
- Priority-sorted available tasks (High/Medium/Low)
- Coordination status and conflict detection
- Task recommendations with rationale
- Completion status and blocked tasks
- Structured decision summary

**DO NOT** create new task analysis scripts - use the existing one at `meta/scripts/analyze-agent-tasks.sh`

---

**Note**: This is a modularized version of CLAUDE.md. Full documentation is available in the imported modules above. The original file is backed up as CLAUDE.md.backup if restoration is needed.

[Mode:Meta]