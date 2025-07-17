# [Task Title]

## Task Overview
**Priority**: [High/Medium/Low]
**Assigned Mode**: [bevy|entt|flecs|console|meta|orchestrator]
**Created By**: [Creator]
**Status**: [pending|in_progress|completed]

## Coordination Metadata
**Parallel Safety**: [SAFE|UNSAFE|CONDITIONAL]
**Classification**: [Isolated|Implementation-Specific|Shared Resource|Sequential]
**Estimated Duration**: [minutes]
**Conflicts With**: [List of task files that cannot run simultaneously]
**Depends On**: [List of task files that must complete first]
**Modifies Files**: [List of file paths that will be modified]
**Reads Files**: [List of file paths that will be read]
**Locked Resources**: [List of shared resources that need locking]
**Parallel Compatible**: [true|false]

## Objective
[Clear description of what should be accomplished]

## Context
[Background information and project context]

## Requirements
[Detailed requirements and specifications]

## Implementation Steps
[Step-by-step implementation approach]

## Acceptance Criteria
[Checklist of completion criteria]

## Agent Constraints
- Work exclusively within [scope] directory
- Maintain mode boundaries and constraints
- End response with [Mode:[ModeName]] declaration
- [Other mode-specific constraints]

## Expected Deliverables
[List of expected outputs and artifacts]

## Technical Considerations
[Technical details, constraints, and considerations]

## Success Indicators
[How to measure successful completion]

## Notes
[Additional context, warnings, or special instructions]

---

## Coordination System Usage

### Before Starting This Task
1. **Check Dependencies**: Verify all tasks in "Depends On" are completed
2. **Check Conflicts**: Verify no tasks in "Conflicts With" are currently active
3. **Check File Locks**: Verify all files in "Modifies Files" are available
4. **Register Agent**: Add entry to active-agents.json
5. **Acquire Locks**: Lock all files that will be modified

### During Task Execution
1. **Update Heartbeat**: Every 5 minutes update heartbeat in active-agents.json
2. **Update Status**: Update status at key milestones
3. **Monitor Conflicts**: Watch for new conflicts that arise
4. **Maintain Locks**: Extend lock expiration if task runs longer than expected

### After Task Completion
1. **Update Status**: Mark task as completed in active-agents.json
2. **Release Locks**: Remove all file locks from file-locks.json
3. **Cleanup Agent**: Remove agent entry from active-agents.json
4. **Update Dependencies**: Notify any tasks that depend on this one