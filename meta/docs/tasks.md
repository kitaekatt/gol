# Task Management System

## Task Management Quick Reference

### Current Implementation Status
- **Bevy**: Check gol-bevy/tasks/ for available tasks
- **EnTT**: Check gol-entt/tasks/ for available tasks
- **Flecs**: Check gol-flecs/tasks/ for available tasks
- **Console**: Check gol-console-client/tasks/ for available tasks
- **Meta**: Check meta/tasks/ for available tasks

### Task Priority Guidelines
1. **High Priority**: setup-project.md, critical blockers, dependencies
2. **Medium Priority**: implement-ecs-components.md, implement-game-systems.md
3. **Lower Priority**: implement-api-endpoints.md, optimization tasks

### Task Completion Protocol
- Mark tasks complete IMMEDIATELY after finishing (don't batch completions)
- Only have ONE task in_progress at any time
- Complete current tasks before starting new ones
- Remove tasks that are no longer relevant from the list entirely

### Task Completion Requirements
- ONLY mark a task as completed when you have FULLY accomplished it
- If you encounter errors, blockers, or cannot finish, keep the task as in_progress
- When blocked, create a new task describing what needs to be resolved
- Never mark a task as completed if:
  - Tests are failing
  - Implementation is partial
  - You encountered unresolved errors
  - You couldn't find necessary files or dependencies

### Task Breakdown Guidelines
- Create specific, actionable items
- Break complex tasks into smaller, manageable steps
- Use clear, descriptive task names
- Focus on single responsibility per task