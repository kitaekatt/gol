# Coordination Protocol Documentation

## Overview
The coordination protocol enables multiple agents to work in parallel safely by managing task dependencies, file conflicts, and resource sharing.

## Core Components

### 1. Active Agent Tracking
File: `active-agents.json`
Purpose: Track which agents are currently working on which tasks

Schema:
```json
{
  "agents": {
    "agent_id": {
      "mode": "bevy|entt|flecs|console|meta|orchestrator",
      "current_task": "task_filename.md",
      "task_started": "2024-01-01T12:00:00Z",
      "estimated_completion": "2024-01-01T13:00:00Z",
      "locked_files": ["path/to/file1", "path/to/file2"],
      "status": "active|paused|completing",
      "parallel_compatible": true,
      "priority": "high|medium|low"
    }
  },
  "last_updated": "2024-01-01T12:00:00Z",
  "schema_version": "1.0",
  "coordination_enabled": true
}
```

### 2. File Lock Management
File: `file-locks.json`
Purpose: Manage exclusive access to shared files and directories

Schema:
```json
{
  "locks": {
    "file_path": {
      "locked_by": "agent_id",
      "lock_type": "read|write|exclusive",
      "locked_at": "2024-01-01T12:00:00Z",
      "expires_at": "2024-01-01T13:00:00Z",
      "task": "task_filename.md",
      "priority": "high|medium|low"
    }
  },
  "last_updated": "2024-01-01T12:00:00Z",
  "schema_version": "1.0",
  "lock_timeout_minutes": 60
}
```

### 3. Task Metadata
Purpose: Enhanced task files with coordination information

Required metadata in each task:
- Task Classification (Isolated/Implementation-Specific/Shared Resource/Sequential)
- Conflicts With (tasks that cannot run in parallel)
- Depends On (prerequisite tasks)
- Modifies Files (files/directories task will modify)
- Reads Files (files/directories task will read)
- Parallel Safe (boolean)
- Estimated Duration (minutes)
- File Lock Priority (High/Medium/Low)

## Coordination Workflows

### 1. Task Start Protocol
1. **Agent Mode Activation**: Agent enters specific mode (bevy, entt, flecs, console, meta)
2. **Task Selection**: Agent selects next task from its tasks/ directory
3. **Coordination Check**: Agent calls coordination validation
4. **Conflict Detection**: System checks for file conflicts and dependencies
5. **Lock Acquisition**: Agent acquires necessary file locks
6. **Status Update**: Agent updates active-agents.json with current task
7. **Task Execution**: Agent begins task execution
8. **Status Reporting**: Periodic status updates during execution

### 2. Conflict Detection Logic
```
function canExecuteTask(agent, task):
    // Check dependencies
    for dependency in task.depends_on:
        if not isTaskCompleted(dependency):
            return false, "Dependency not met: " + dependency
    
    // Check file conflicts
    for file in task.modifies_files:
        if isFileLocked(file) and lockOwner(file) != agent:
            return false, "File locked by another agent: " + file
    
    // Check task conflicts
    for activeAgent in getActiveAgents():
        if activeAgent.current_task in task.conflicts_with:
            return false, "Conflicting task active: " + activeAgent.current_task
    
    return true, "Task can be executed"
```

### 3. Lock Management
```
function acquireLock(agent, file_path, lock_type, priority):
    // Check if file is already locked
    existing_lock = getLock(file_path)
    if existing_lock:
        if existing_lock.priority < priority:
            // Higher priority can override
            releaseLock(existing_lock)
        else:
            return false, "File locked by higher priority task"
    
    // Acquire lock
    lock = {
        "locked_by": agent,
        "lock_type": lock_type,
        "locked_at": getCurrentTime(),
        "expires_at": getCurrentTime() + lock_timeout,
        "priority": priority
    }
    setLock(file_path, lock)
    return true, "Lock acquired"
```

### 4. Task Completion Protocol
1. **Task Completion**: Agent completes task execution
2. **Status Update**: Update active-agents.json with completion
3. **Lock Release**: Release all file locks acquired by agent
4. **Cleanup**: Remove task file and update task tracking
5. **Orchestrator Notification**: Report completion to orchestrator if required
6. **Next Task**: Agent selects next task or enters idle state

## Agent Integration

### Mode Template Updates
Each agent mode template should include:
```
COORDINATION PROTOCOL:
1. Before task execution: Call coordination validation
2. During task execution: Maintain lock status
3. After task completion: Release locks and update status
4. On agent exit: Clean up all locks and status entries
```

### Coordination API
Agents interact with coordination through standardized functions:
- `validateTaskExecution(agent, task)` - Check if task can be executed
- `acquireFileLocks(agent, files, priority)` - Acquire necessary locks
- `updateAgentStatus(agent, status)` - Update agent status
- `releaseAllLocks(agent)` - Release all locks held by agent
- `getParallelTasks(agent)` - Get list of tasks safe for parallel execution

## Error Handling

### Stale Lock Detection
- Locks expire after 60 minutes by default
- Periodic cleanup process removes expired locks
- Agents can request lock override for expired locks

### Agent Failure Recovery
- Detect agent failures through status timeout
- Automatic lock cleanup for failed agents
- Task rollback procedures for interrupted work
- Graceful degradation to single-agent mode

### Conflict Resolution
- Priority-based conflict resolution
- Queuing system for conflicting tasks
- Alternative task suggestions when conflicts occur
- Fallback to sequential execution if needed

## Performance Considerations

### Lightweight Implementation
- File-based coordination (no complex databases)
- JSON files for simplicity and readability
- Minimal coordination overhead
- Fast conflict detection algorithms

### Scalability
- Designed for current 4 implementation agents
- Extensible to additional agents
- Efficient file locking mechanisms
- Low memory footprint

## Testing and Validation

### Coordination Testing
- Test parallel execution of compatible tasks
- Validate conflict detection accuracy
- Test lock acquisition and release
- Verify graceful degradation scenarios

### Performance Testing
- Measure coordination overhead
- Test with multiple active agents
- Validate file locking performance
- Measure conflict resolution speed

## Future Enhancements

### Advanced Features
- Task priority scheduling
- Dynamic load balancing
- Resource utilization tracking
- Predictive conflict detection

### Integration Improvements
- IDE integration for coordination status
- Real-time coordination dashboard
- Advanced conflict resolution strategies
- Automated performance optimization