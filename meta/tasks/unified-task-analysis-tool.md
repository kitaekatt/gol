# Unified Task Analysis Tool

## Task Overview
**Priority**: Medium
**Assigned Mode**: Meta
**Estimated Duration**: 30 minutes
**Status**: COMPLETED

## Objective
Enhance the task analysis script to accept multiple agent contexts and create a unified project-wide report, reducing the need for multiple tool calls during orchestrator task assessment.

## Current State
The `analyze-agent-tasks.sh` script currently:
- Accepts only single agent mode (bevy, entt, flecs, console, meta)
- Requires 5 separate tool calls to assess all agent tasks
- Produces individual reports per agent

## Requirements

### Core Enhancement
- Accept multiple agent contexts: `analyze-agent-tasks.sh all` or `analyze-agent-tasks.sh bevy entt flecs`
- Generate unified project-wide task priority report
- Maintain existing single-agent functionality

### Output Format
```
=== UNIFIED PROJECT TASK ANALYSIS ===
Coordination Status: CLEAR
Total Active Agents: 0

=== HIGHEST PRIORITY TASKS (Cross-Agent) ===
🔴 HIGH PRIORITY
  • [ENTT] cpp-environment-assessment.md [30 min] - Foundation work
  • [FLECS] setup-flecs-project.md [60 min] - Foundation work  
  • [CONSOLE] bevy-integration-demo.md [45 min] - Validation work

=== AGENT SUMMARY ===
- BEVY: 0 tasks available
- ENTT: 5 tasks available (4 blocked by dependencies)
- FLECS: 1 task available
- CONSOLE: 1 task available
- META: 0 tasks available

=== RECOMMENDED ORCHESTRATOR ACTION ===
Delegate cpp-environment-assessment.md to EnTT agent (shortest duration, highest impact)
```

### Technical Implementation
- Modify `analyze-agent-tasks.sh` to accept "all" parameter
- Create unified sorting logic across all agent priorities
- Maintain coordination status checking across all agents
- Provide orchestrator-specific recommendations

## Acceptance Criteria
- [x] Script accepts `analyze-agent-tasks.sh all` parameter
- [x] Generates unified cross-agent task priority report
- [x] Maintains backward compatibility with single-agent mode
- [x] Provides orchestrator-specific recommendations
- [x] Reduces orchestrator tool calls from 5 to 1 for full assessment

## Context
This enhancement addresses the orchestrator's need for efficient project-wide task assessment. Currently, determining "what's next" requires 5 separate tool calls, which is inefficient for coordination workflow.

## Dependencies
- Existing `analyze-agent-tasks.sh` script
- Coordination system scripts
- Task file structure across all agents

## Notes
This is a meta-improvement that directly enhances orchestrator efficiency and reduces cognitive load during project coordination.