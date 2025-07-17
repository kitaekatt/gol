# [Task Title]

## Task Overview
**Priority**: [High/Medium/Low]
**Assigned Mode**: [Target Agent Mode]
**Created By**: Orchestrator sub-task orchestration
**Status**: [pending|active|completed]

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
[Clear description of what the agent should accomplish]

## Context
[Background information and constraints]

## Requirements
[Specific deliverables and success criteria]

## Agent Constraints
- Work exclusively within [mode] directory scope
- Maintain mode boundaries and constraints
- End response with [Mode:[AgentMode]] declaration
- [Other mode-specific constraints]

## Expected Deliverables
[List of expected outputs]

## Acceptance Criteria
[How to determine task completion]

## Orchestrator Notes
[Special instructions for orchestrator coordination]

---

## Sub-Task Orchestration Protocol

### Task Creation Process
1. **Orchestrator Analysis**: Determine appropriate agent mode for task
2. **Coordination Check**: Verify parallel safety and dependencies
3. **Task File Creation**: Create formal task file with coordination metadata
4. **Agent Delegation**: Use Task tool to delegate work to appropriate agent
5. **Communication Flow**: Display full orchestrator ↔ agent communication

### Communication Flow Template
```
1. ORCHESTRATOR SENDS PROMPT TO [MODE] AGENT:
   [Full prompt with mode constraints and task details]

2. EXECUTING SUBTASK:
   Task([description])
   ⎿ [Agent response with full analysis and results]

3. ORCHESTRATOR RECEIVES SUBTASK RESPONSE:
   [Agent findings and recommendations]

4. ORCHESTRATOR ANALYSIS:
   [Orchestrator processing of agent results]

5. ORCHESTRATOR DECISION:
   [Next steps and actions based on agent work]
```

### Coordination Requirements
- **Task File Mandatory**: Always create corresponding task file
- **Mode Compliance**: Sub-tasks must operate within assigned agent mode constraints
- **High Fidelity**: Full communication flow must be visible to user
- **Proper Delegation**: Orchestrator delegates rather than performing restricted work directly
- **Parallel Safety**: Consider parallel execution opportunities when creating tasks