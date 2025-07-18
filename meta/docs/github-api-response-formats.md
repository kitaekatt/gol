# GitHub API Response Formats for AI Consumption

## Overview
This document specifies the structured information formats that AI agents receive when interacting with GitHub Issues through the github-core-ops.sh script. These formats are designed to be easily parseable and provide all necessary information for task coordination.

## Core Response Formats

### 1. Task List (Summary Format)
**Command**: `github_list_tasks [status] [agent] [priority] [summary]`
**Use Case**: AI agents need to see available tasks quickly

```
=== AVAILABLE TASKS ===
Task #1: Test AI Task Management [OPEN] - Assignee: kitaekatt - Labels: status:in-progress, priority:high, agent:meta
Task #2: Implement Bevy ECS Components [OPEN] - Assignee: None - Labels: status:todo, priority:high, agent:bevy
```

**Parsing Pattern**: 
- `Task #(\d+): (.+) \[(\w+)\] - Assignee: (.+) - Labels: (.+)`
- Groups: issue_number, title, state, assignee, labels

### 2. Task List (JSON Format)
**Command**: `github_list_tasks [status] [agent] [priority] json`
**Use Case**: Programmatic processing of multiple tasks

```json
[
  {
    "number": 1,
    "title": "Test AI Task Management",
    "labels": [
      {"name": "status:in-progress", "color": "0052cc"},
      {"name": "priority:high", "color": "b60205"},
      {"name": "agent:meta", "color": "fd79a8"}
    ],
    "assignees": [{"login": "kitaekatt"}],
    "state": "OPEN",
    "createdAt": "2025-07-17T19:43:38Z",
    "updatedAt": "2025-07-17T19:44:57Z"
  }
]
```

### 3. Task Details (Detailed Format)
**Command**: `github_get_task <issue_number> [detailed]`
**Use Case**: AI agents need full context for task work

```
=== TASK #1 DETAILS ===
Title: Test AI Task Management
Status: OPEN
Assignees: kitaekatt
Labels: status:in-progress, priority:high, agent:meta
Created: 2025-07-17T19:43:38Z
Updated: 2025-07-17T19:44:57Z

=== DESCRIPTION ===
## Objective
Test GitHub API core operations for AI task management

## Requirements
- Validate task creation functionality
- Test status transitions
- Verify AI parseability

## Acceptance Criteria
- [ ] Task created successfully
- [ ] Labels applied correctly
- [ ] AI can parse task information

=== COMMENTS (1) ===
[2025-07-17T19:44:57Z] kitaekatt: Testing GitHub API core operations validation. Status transitions and assignment working correctly.
```

**Parsing Structure**:
- Header section with metadata
- Description section with structured markdown
- Comments section with chronological updates

### 4. Task Status Query
**Command**: `github_query_status <issue_number>`
**Use Case**: Quick coordination and assignment checks

```
=== TASK STATUS QUERY ===
Issue: #1 - Test AI Task Management
State: OPEN
Status: in-progress
Priority: high
Agent: meta
Assignee: kitaekatt
Coordination: Assigned
```

**Key Fields for AI**:
- **Status**: Current workflow state (todo, in-progress, done, blocked)
- **Priority**: Task priority level (high, medium, low)
- **Agent**: Assigned agent/mode (bevy, entt, flecs, console, meta, orchestrator)
- **Assignee**: GitHub user assignment
- **Coordination**: Available/Assigned status

### 5. AI Coordination Summary
**Command**: `github_get_ai_summary [agent] [status]`
**Use Case**: AI agents need high-level project overview

```
=== AI TASK COORDINATION SUMMARY ===
Generated: 2025-07-17T14:45:10-05:00

=== TASK STATUS OVERVIEW ===
todo: 0 tasks
in-progress: 1 tasks
done: 0 tasks
blocked: 0 tasks

=== FILTERED TASK LIST ===
=== AVAILABLE TASKS ===
Task #1: Test AI Task Management [OPEN] - Assignee: kitaekatt - Labels: status:in-progress, priority:high, agent:meta

=== ASSIGNMENT OVERVIEW ===
bevy: 0 tasks assigned
entt: 0 tasks assigned
flecs: 0 tasks assigned
console: 0 tasks assigned
meta: 1 tasks assigned
orchestrator: 0 tasks assigned
```

## Label Structure for AI Parsing

### Status Labels
- `status:todo` - Task ready to start
- `status:in-progress` - Task currently being worked on
- `status:done` - Task completed
- `status:blocked` - Task blocked by dependencies

### Priority Labels
- `priority:high` - Critical or blocking tasks
- `priority:medium` - Standard priority tasks
- `priority:low` - Nice-to-have or cleanup tasks

### Agent Labels
- `agent:bevy` - Bevy ECS implementation agent
- `agent:entt` - EnTT ECS implementation agent
- `agent:flecs` - Flecs ECS implementation agent
- `agent:console` - Console client agent
- `agent:meta` - Meta-development agent
- `agent:orchestrator` - Orchestrator coordination agent

## AI Parsing Strategies

### 1. Extracting Label Information
```bash
# Extract status from labels
echo "$labels" | grep -o 'status:[^,]*' | cut -d: -f2

# Extract priority from labels
echo "$labels" | grep -o 'priority:[^,]*' | cut -d: -f2

# Extract agent from labels
echo "$labels" | grep -o 'agent:[^,]*' | cut -d: -f2
```

### 2. JSON Processing with jq
```bash
# Get all high priority tasks
jq '.[] | select(.labels[].name | contains("priority:high"))'

# Extract task assignments
jq '.[] | {number, title, assignee: (.assignees[0].login // "None")}'

# Filter by agent
jq '.[] | select(.labels[].name | contains("agent:meta"))'
```

### 3. Task Structure Validation
AI agents should verify:
- Task has status label
- Task has priority label  
- Task has agent label (if assigned)
- Task description contains structured sections (## Objective, ## Requirements, ## Acceptance Criteria)

## Response Format Guarantees

### Consistency
- All commands provide consistent field names and structures
- Status values are always lowercase with hyphens
- Dates are in ISO 8601 format
- Labels follow `category:value` pattern

### Completeness
- Every task response includes sufficient information for AI decision-making
- Missing information is explicitly indicated (e.g., "Assignee: None")
- Error states are clearly communicated

### Parseability
- Human-readable format maintains structure for regex parsing
- JSON format provides programmatic access
- Mixed format commands separate structured data clearly

## Error Handling

### Common Error Responses
```bash
❌ Issue number required
❌ Invalid status: invalid. Valid options: todo in-progress done blocked
❌ Title and body are required
```

### AI Error Recovery
AI agents should:
1. Validate input parameters before calling commands
2. Check for error messages in stderr
3. Fall back to manual task creation if automation fails
4. Report errors to orchestrator for intervention

## Integration with Flat File System

### Equivalence Mapping
- GitHub Issues ↔ Task files (.md)
- Issue labels ↔ Task metadata
- Issue comments ↔ Progress updates
- Issue state ↔ File location (tasks/ vs completed/)

### Migration Strategy
AI agents can:
1. Query GitHub Issues to understand available work
2. Use same coordination patterns as flat files
3. Report progress through issue comments
4. Close issues when tasks complete

This response format specification ensures AI agents can seamlessly interact with GitHub Issues while maintaining the same task management capabilities they have with flat files.