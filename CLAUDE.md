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
- **"Update documentation"** - Execute batch documentation update process (captures all session learnings)
- **"Begin shutdown"** - Initiate proper shutdown sequence with quality gates
- **"Is documentation up to date?"** - Verify documentation currency before shutdown
- **"Session status"** - Show current task status and shutdown readiness

### GitHub Issues Commands (NEW)
- **"GitHub list tasks"** - Show GitHub Issues for current agent mode
- **"GitHub claim [issue-number]"** - Assign GitHub Issue to current agent
- **"GitHub complete [issue-number]"** - Complete and close GitHub Issue
- **"GitHub continue development"** - Continue work using GitHub Issues system
- **"GitHub update status [issue-number] [status]"** - Change GitHub Issue status
- **"Create GitHub issue [title]"** - Create new GitHub Issue for current agent

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
8. **Orchestration Planning**: Always start orchestration with comprehensive plan sharing before spawning agents

## Workflow Optimization Notes

- **Tool Efficiency Strategies**: 
  * When reflecting on performance, consider cases where multiple tool calls were required to accomplish a task, and it is likely this pattern will repeat.
  * Consider solutions like making dedicated bash scripts that can quickly and with a single tool call give you the information you need.
  * When executing work, consider if a bash script will allow you to accomplish work more quickly.
  * These scripts must be in a tmp directory of some sort and be deleted as they are task-specific.

- **Orchestration Planning Protocol**:
  * **Plan First**: Always start orchestration with comprehensive plan sharing before spawning agents
  * **Coordination Options**: Present parallel vs sequential task coordination and agent spawning vs manual coordination options
  * **Outcome Description**: Describe benefits/outcomes of proposed tasks and what they enable next
  * **Completion Reporting**: When grouped tasks complete, summarize benefits achieved and next task group opportunities
  * **Decision Points**: Clear communication about coordination strategy choices for user approval

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

## GitHub Issues Task System - NEW WORKFLOW OPTION

**IMPORTANT**: Agents can now choose between traditional file-based tasks and GitHub Issues-based tasks:

### GitHub Issues Task Analysis Scripts:
- **Bevy**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh bevy")`
- **EnTT**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh entt")`  
- **Flecs**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh flecs")`
- **Console**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh console")`
- **Meta**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh meta")`
- **All Agents**: `Bash(command="bash meta/scripts/github-analyze-agent-tasks.sh all")`

### GitHub Issues Coordination Scripts:
- **Coordination Check**: `Bash(command="bash meta/scripts/github-coordination-check.sh [mode] work_on_task")`
- **Claim Issue**: `Bash(command="bash meta/scripts/github-enforce-coordination.sh [mode] [issue-number]")`
- **Complete Task**: `Bash(command="bash meta/scripts/github-task-completion.sh [mode] [issue-number] [message]")`

### GitHub Issues Core Operations:
- **List Issues**: `Bash(command="bash meta/scripts/github-core-ops.sh list [status] [agent] [priority]")`
- **Get Issue**: `Bash(command="bash meta/scripts/github-core-ops.sh get [issue-number]")`
- **Create Issue**: `Bash(command="bash meta/scripts/github-core-ops.sh create \"[title]\" \"[body]\" [priority] [agent]")`
- **Update Status**: `Bash(command="bash meta/scripts/github-core-ops.sh change-status [issue-number] [status]")`
- **Add Comment**: `Bash(command="bash meta/scripts/github-core-ops.sh comment [issue-number] \"[comment]\"")`

### Task System Selection Guide:
- **Traditional File-Based**: Use for established workflows, file-based task management
- **GitHub Issues**: Use for better collaboration, issue tracking, and project visibility
- **Both systems** can operate in parallel during transition period
- **Mode templates** include instructions for both systems

### GitHub Issues Benefits:
- **Better Visibility**: Tasks visible in GitHub UI and project boards
- **Enhanced Collaboration**: Comments, assignees, labels for coordination
- **Integrated Workflow**: Links directly with code commits and PRs
- **Project Management**: Native GitHub project management features
- **Audit Trail**: Complete history of task progress and decisions

---

**Note**: This is a modularized version of CLAUDE.md. Full documentation is available in the imported modules above. The original file is backed up as CLAUDE.md.backup if restoration is needed.

[Mode:Meta]