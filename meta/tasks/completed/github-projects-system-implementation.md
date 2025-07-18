# GitHub Projects System Implementation

## Task Overview
**Priority**: High
**Assigned Mode**: Meta
**Phase**: 3 (Integration)

## Objective
Build a complete GitHub Projects system that replicates the capabilities of our current flat file task system. This system should be sufficient for our task management going forward and provide equivalent functionality to what AI agents currently use.

## Context
Phase 2 has established core GitHub API operations. Now we need to build the complete system that integrates with our agent modes and command structure, providing a full replacement for the flat file task system.

## Requirements

### Agent Mode Integration
Replicate current agent capabilities using GitHub Issues:
- **Task Discovery**: Equivalent to `analyze-agent-tasks.sh` for each mode
- **Task Coordination**: Equivalent to coordination enforcement and claim management
- **Task Execution**: Support for "Continue development" and task completion workflows
- **Status Reporting**: Support for "Update the orchestrator" via issue comments

### Command System Integration
Update all relevant commands to support GitHub Issues:
- **"List tasks"**: Show available GitHub Issues for current agent mode
- **"Continue development"**: Query and work on GitHub Issues
- **"Update the orchestrator"**: Post progress updates as issue comments
- **"Session status"**: Include GitHub Issues task status

### Agent Mode Scripts
Create GitHub Issues versions of existing coordination scripts:
- `github-analyze-agent-tasks.sh` for each mode (bevy, entt, flecs, console, meta)
- `github-enforce-coordination.sh` for task claiming
- `github-coordination-check.sh` for coordination status
- `github-task-completion.sh` for task completion workflow

### Documentation Updates
Update all mode templates and instructions:
- Add GitHub Issues workflow to each mode template
- Update CLAUDE.md with GitHub Issues command patterns
- Create documentation for GitHub Issues agent operations
- Maintain parallel documentation for both systems

## Implementation Tasks

### 1. Agent Query System
Create mode-specific task query scripts:
```bash
# github-analyze-agent-tasks.sh [mode]
# Equivalent functionality to current analyze-agent-tasks.sh
# Outputs: priority-sorted available tasks with coordination status
```

### 2. Coordination System
Implement GitHub Issues coordination:
```bash
# github-enforce-coordination.sh [mode] [issue-number]
# Claims task by assigning to agent mode
# Updates issue status to in-progress

# github-coordination-check.sh [mode] [action]
# Checks coordination status before task work
# Equivalent to mandatory-coordination-check.sh
```

### 3. Command Integration
Update core commands to support GitHub Issues:
- Modify "Continue development" to work with GitHub Issues
- Update "List tasks" to query GitHub Issues with proper filtering
- Enhance "Update the orchestrator" to post issue comments
- Add GitHub Issues status to "Session status" command

### 4. Mode Template Updates
Update all mode activation templates:
- Add GitHub Issues workflow instructions
- Include coordination requirements using GitHub Issues
- Update task discovery patterns
- Maintain mode boundary enforcement

### 5. Documentation Integration
Update project documentation:
- Add GitHub Issues workflow to CLAUDE.md
- Update mode documentation with GitHub Issues patterns
- Create GitHub Issues command reference
- Document transition approach for users

### 6. **FIRST TASK MIGRATION AND EXECUTION**
Create and execute the first real task using GitHub Issues:
- User creates a real development task as GitHub Issue
- AI agent queries and claims the task using new GitHub Issues system
- Complete the task workflow end-to-end using GitHub Issues
- Document lessons learned and system refinements needed

## Agent Constraints
- Work exclusively within meta/ directory scope
- Maintain compatibility with existing flat file system during implementation
- Focus on functional equivalence with current task management capabilities
- Ensure GitHub Issues system can operate independently

## Expected Deliverables
1. **Complete Agent Query System**: Scripts for all modes to discover and prioritize GitHub Issues
2. **Full Coordination System**: GitHub Issues-based coordination matching current functionality
3. **Integrated Command System**: All user commands support GitHub Issues workflow
4. **Updated Mode Templates**: All agent modes can operate using GitHub Issues
5. **Documentation Updates**: Complete documentation for GitHub Issues workflow
6. ****VALIDATED SYSTEM**: Successfully completed first real task using GitHub Issues end-to-end**

## Acceptance Criteria
- [ ] All agent modes can discover and claim GitHub Issues tasks
- [ ] Coordination system prevents conflicts and manages assignment
- [ ] "Continue development" command works with GitHub Issues
- [ ] "List tasks" shows appropriate GitHub Issues for each mode
- [ ] "Update the orchestrator" posts meaningful progress updates
- [ ] Mode boundaries are maintained with GitHub Issues
- [ ] **First real task completed successfully using GitHub Issues system**
- [ ] System is ready for independent operation without flat files
- [ ] Documentation enables smooth user transition

## Notes
This task creates the complete GitHub Projects system for task management. The key milestone is successfully completing the first real development task using GitHub Issues, proving the system works end-to-end.

Phase 3 culminates with the **FIRST TASK MIGRATION AND EXECUTION** - this is where we prove the GitHub Issues system is ready for real work. The user will create a development task as a GitHub Issue, and an AI agent will complete it using the new system, demonstrating full functionality.

Success means the GitHub Issues system can fully replace the flat file system for task management, with AI agents operating seamlessly using GitHub's native features.