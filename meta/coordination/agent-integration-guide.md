# Agent Integration Guide

## Overview
This guide explains how to integrate the coordination system into agent workflows to enable safe parallel execution.

## Integration Steps

### 1. Agent Mode Template Updates

Each agent mode template needs to be updated with coordination protocol integration. Here's the template for each mode:

#### Bevy Mode Template Update
```markdown
BEVY MODE ACTIVATED

I am now operating as a Bevy implementation agent with the following active constraints:
- Work exclusively on tasks in gol-bevy/tasks/
- Implement Bevy-specific ECS architecture
- Full read/write access restricted to gol-bevy/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files

**COORDINATION PROTOCOL INTEGRATION**:
- Before task execution: Validate task can be executed safely
- During task execution: Maintain coordination status and file locks
- After task completion: Release locks and update coordination state
- On agent exit: Clean up all coordination state

Current focus:
1. Check gol-bevy/tasks/ for available tasks
2. Validate task compatibility with active agents
3. Acquire necessary file locks for task execution
4. Follow task instructions and requirements exactly
5. Maintain coordination status throughout execution
6. Validate implementation against acceptance criteria
7. Release locks and report completion to orchestrator
8. Escalate blockers to orchestrator if encountered
```

#### EnTT Mode Template Update
```markdown
ENTT MODE ACTIVATED

I am now operating as an EnTT implementation agent with the following active constraints:
- Work exclusively on tasks in gol-entt/tasks/
- Implement EnTT-specific ECS architecture
- Full read/write access restricted to gol-entt/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files

**COORDINATION PROTOCOL INTEGRATION**:
- Before task execution: Validate task can be executed safely
- During task execution: Maintain coordination status and file locks
- After task completion: Release locks and update coordination state
- On agent exit: Clean up all coordination state

Current focus:
1. Check gol-entt/tasks/ for available tasks
2. Validate task compatibility with active agents
3. Acquire necessary file locks for task execution
4. Follow task instructions and requirements exactly
5. Maintain coordination status throughout execution
6. Validate implementation against acceptance criteria
7. Release locks and report completion to orchestrator
8. Escalate blockers to orchestrator if encountered
```

#### Flecs Mode Template Update
```markdown
FLECS MODE ACTIVATED

I am now operating as a Flecs implementation agent with the following active constraints:
- Work exclusively on tasks in gol-flecs/tasks/
- Implement Flecs-specific ECS architecture
- Full read/write access restricted to gol-flecs/ directory only
- Cannot modify other implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files

**COORDINATION PROTOCOL INTEGRATION**:
- Before task execution: Validate task can be executed safely
- During task execution: Maintain coordination status and file locks
- After task completion: Release locks and update coordination state
- On agent exit: Clean up all coordination state

Current focus:
1. Check gol-flecs/tasks/ for available tasks
2. Validate task compatibility with active agents
3. Acquire necessary file locks for task execution
4. Follow task instructions and requirements exactly
5. Maintain coordination status throughout execution
6. Validate implementation against acceptance criteria
7. Release locks and report completion to orchestrator
8. Escalate blockers to orchestrator if encountered
```

#### Console Mode Template Update
```markdown
CONSOLE MODE ACTIVATED

I am now operating as a console client agent with the following active constraints:
- Work exclusively on tasks in gol-console-client/tasks/
- Implement gRPC client for all three server implementations
- Create terminal UI for Game of Life visualization and control
- Full read/write access restricted to gol-console-client/ directory only
- Cannot modify server implementations or create cross-implementation tasks
- Report completion to orchestrator and delete completed task files

**COORDINATION PROTOCOL INTEGRATION**:
- Before task execution: Validate task can be executed safely
- During task execution: Maintain coordination status and file locks
- After task completion: Release locks and update coordination state
- On agent exit: Clean up all coordination state

Current focus:
1. Check gol-console-client/tasks/ for available tasks
2. Validate task compatibility with active agents
3. Acquire necessary file locks for task execution
4. Implement gRPC client connectivity to Bevy, EnTT, and Flecs servers
5. Create interactive terminal interface for simulation control
6. Maintain coordination status throughout execution
7. Validate client functionality with all server implementations
8. Release locks and report completion to orchestrator
9. Escalate blockers to orchestrator if encountered
```

#### Meta Mode Template Update
```markdown
META MODE ACTIVATED

I am now operating as a meta-development agent with the following active constraints:
- Work exclusively on tasks in meta/tasks/
- Implement memory, learning, and workflow improvements
- Optimize documentation and development processes
- Full read/write access to meta/ directory
- Write access to any directory with careful coordination
- Self-check before modifying files in other modes' directories
- Escalate to orchestrator when cross-mode coordination needed

**COORDINATION PROTOCOL INTEGRATION**:
- Before task execution: Validate task can be executed safely
- During task execution: Maintain coordination status and file locks
- After task completion: Release locks and update coordination state
- On agent exit: Clean up all coordination state
- Special handling for shared resource modifications

Current focus:
1. Check meta/tasks/ for available tasks
2. Validate task compatibility with active agents
3. Acquire necessary file locks for task execution
4. Implement memory and learning improvements
5. Optimize documentation and workflow processes
6. Maintain coordination status throughout execution
7. Enhance development experience and productivity
8. Coordinate improvements affecting multiple modes
9. Self-evaluate: Should orchestrator delegate to appropriate agent instead?
10. Release locks and report completion to orchestrator
```

### 2. Coordination Workflow Implementation

#### Pre-Task Execution
```bash
# Agent identifier (unique per agent instance)
AGENT_ID="$(uname -n)-$(date +%s)-$$"
MODE="bevy"  # or entt, flecs, console, meta
TASK_FILE="$1"  # Path to task file

# Source coordination utilities
source /mnt/c/dev/gol/meta/coordination/coordination-utils.sh

# Validate task can be executed
if ! validate_task_execution "$AGENT_ID" "$TASK_FILE"; then
    echo "Task cannot be executed due to coordination constraints"
    # Suggest alternative tasks
    suggest_alternative_tasks "$AGENT_ID" "$MODE"
    exit 1
fi

# Acquire necessary file locks
if ! acquire_file_locks "$AGENT_ID" "$TASK_FILE"; then
    echo "Failed to acquire necessary file locks"
    # Wait and retry or suggest alternatives
    handle_lock_failure "$AGENT_ID" "$TASK_FILE"
    exit 1
fi

# Update agent status to active
update_agent_status "$AGENT_ID" "$MODE" "$TASK_FILE" "active"

# Set up cleanup on exit
trap 'coordination_cleanup_on_exit "$AGENT_ID"' EXIT INT TERM
```

#### During Task Execution
```bash
# Periodic status updates (every 15 minutes)
function periodic_status_update() {
    local agent_id="$1"
    local mode="$2"
    local task_file="$3"
    
    while true; do
        sleep 900  # 15 minutes
        update_agent_status "$agent_id" "$mode" "$task_file" "active"
    done
}

# Start background status updates
periodic_status_update "$AGENT_ID" "$MODE" "$TASK_FILE" &
STATUS_PID=$!

# Ensure cleanup of background process
trap 'kill $STATUS_PID 2>/dev/null; coordination_cleanup_on_exit "$AGENT_ID"' EXIT INT TERM
```

#### Post-Task Completion
```bash
# Mark task as completing
update_agent_status "$AGENT_ID" "$MODE" "$TASK_FILE" "completing"

# Perform task cleanup
if [[ -f "$TASK_FILE" ]]; then
    # Move completed task to completed directory
    local task_dir=$(dirname "$TASK_FILE")
    local completed_dir="$task_dir/completed"
    mkdir -p "$completed_dir"
    mv "$TASK_FILE" "$completed_dir/"
fi

# Release all locks and remove agent status
coordination_cleanup_on_exit "$AGENT_ID"

# Kill background status updates
kill $STATUS_PID 2>/dev/null
```

### 3. Coordination State Management

#### Agent Status Updates
```bash
# Function to handle all coordination cleanup on agent exit
function coordination_cleanup_on_exit() {
    local agent_id="$1"
    
    # Release all file locks
    release_agent_locks "$agent_id"
    
    # Remove agent from active status
    remove_agent_status "$agent_id"
    
    # Clean up expired locks and stale agents
    coordination_cleanup
}

# Function to suggest alternative tasks when coordination fails
function suggest_alternative_tasks() {
    local agent_id="$1"
    local mode="$2"
    
    # Find tasks in agent's directory that are parallel-safe
    local task_dir="/mnt/c/dev/gol/gol-$mode/tasks"
    local available_tasks=()
    
    for task_file in "$task_dir"/*.md; do
        if [[ -f "$task_file" ]]; then
            if validate_task_execution "$agent_id" "$task_file" | grep -q "true"; then
                available_tasks+=("$(basename "$task_file")")
            fi
        fi
    done
    
    if [[ ${#available_tasks[@]} -gt 0 ]]; then
        echo "Alternative tasks available:"
        printf "  - %s\n" "${available_tasks[@]}"
    else
        echo "No alternative tasks available. Consider waiting or switching to a different mode."
    fi
}
```

#### Error Handling and Recovery
```bash
# Function to handle lock acquisition failures
function handle_lock_failure() {
    local agent_id="$1"
    local task_file="$2"
    
    echo "Failed to acquire locks for $(basename "$task_file")"
    
    # Check who owns the conflicting locks
    local task_metadata=$(extract_task_metadata "$task_file")
    local modifies_files=$(echo "$task_metadata" | jq -r '.modifies_files[]?')
    
    echo "Conflicting locks:"
    for file in $modifies_files; do
        local lock_info=$(get_lock_info "$file")
        if [[ -n "$lock_info" ]]; then
            echo "  $file: locked by $(echo "$lock_info" | jq -r '.locked_by') for task $(echo "$lock_info" | jq -r '.task')"
        fi
    done
    
    # Suggest waiting or alternative tasks
    suggest_alternative_tasks "$agent_id" "$(get_agent_mode "$agent_id")"
}

# Function to get lock information
function get_lock_info() {
    local file_path="$1"
    local locks_file="/mnt/c/dev/gol/meta/coordination/file-locks.json"
    
    jq -r ".locks[\"$file_path\"]" "$locks_file"
}
```

### 4. Task Selection Enhancement

#### Smart Task Selection
```bash
# Function to select next task considering coordination
function select_next_task() {
    local agent_id="$1"
    local mode="$2"
    local task_dir="/mnt/c/dev/gol/gol-$mode/tasks"
    
    # Get all available tasks
    local all_tasks=()
    for task_file in "$task_dir"/*.md; do
        if [[ -f "$task_file" ]]; then
            all_tasks+=("$task_file")
        fi
    done
    
    # Sort by priority and parallel safety
    local prioritized_tasks=()
    for task_file in "${all_tasks[@]}"; do
        if validate_task_execution "$agent_id" "$task_file" | grep -q "true"; then
            local priority=$(get_task_priority "$task_file")
            prioritized_tasks+=("$priority:$task_file")
        fi
    done
    
    # Sort by priority (high, medium, low)
    IFS=$'\n' sorted_tasks=($(sort -t: -k1,1 <<<"${prioritized_tasks[*]}"))
    
    # Return highest priority task
    if [[ ${#sorted_tasks[@]} -gt 0 ]]; then
        echo "${sorted_tasks[0]#*:}"
    else
        echo ""
    fi
}

# Function to get task priority from metadata
function get_task_priority() {
    local task_file="$1"
    local priority=$(grep "^**Priority**:" "$task_file" | cut -d':' -f2 | xargs)
    echo "$priority"
}
```

### 5. Integration with Existing Commands

#### "Continue Development" Command Enhancement
```bash
# Enhanced continue development with coordination
function continue_development_with_coordination() {
    local mode="$1"
    local agent_id="$(uname -n)-$(date +%s)-$$"
    
    # Select next task considering coordination
    local next_task=$(select_next_task "$agent_id" "$mode")
    
    if [[ -n "$next_task" ]]; then
        echo "Selected task: $(basename "$next_task")"
        
        # Execute task with coordination
        execute_task_with_coordination "$agent_id" "$mode" "$next_task"
    else
        echo "No tasks available for parallel execution."
        echo "Active agents and their tasks:"
        show_active_agents
        suggest_alternative_actions "$mode"
    fi
}
```

#### "List Tasks" Command Enhancement
```bash
# Enhanced list tasks with coordination status
function list_tasks_with_coordination() {
    local mode="$1"
    local agent_id="$(uname -n)-$(date +%s)-$$"
    local task_dir="/mnt/c/dev/gol/gol-$mode/tasks"
    
    echo "Available tasks for $mode mode:"
    echo "================================"
    
    for task_file in "$task_dir"/*.md; do
        if [[ -f "$task_file" ]]; then
            local task_name=$(basename "$task_file")
            local can_execute=$(validate_task_execution "$agent_id" "$task_file")
            local priority=$(get_task_priority "$task_file")
            local parallel_safe=$(get_task_parallel_safety "$task_file")
            
            printf "%-30s | %s | %s | %s\n" \
                "$task_name" \
                "$priority" \
                "$parallel_safe" \
                "$can_execute"
        fi
    done
    
    echo ""
    echo "Legend:"
    echo "  Priority: High/Medium/Low"
    echo "  Parallel Safe: Yes/No"
    echo "  Can Execute: true/false (with reason if false)"
}
```

### 6. Testing and Validation

#### Coordination Testing Scripts
```bash
# Test parallel execution capability
function test_parallel_execution() {
    local agents=("bevy" "entt" "flecs" "console")
    local pids=()
    
    echo "Testing parallel execution of compatible tasks..."
    
    for agent in "${agents[@]}"; do
        {
            local agent_id="test-$agent-$$"
            local task_file=$(select_next_task "$agent_id" "$agent")
            
            if [[ -n "$task_file" ]]; then
                echo "[$agent] Starting task: $(basename "$task_file")"
                execute_task_with_coordination "$agent_id" "$agent" "$task_file"
                echo "[$agent] Completed task: $(basename "$task_file")"
            else
                echo "[$agent] No tasks available"
            fi
        } &
        pids+=($!)
    done
    
    # Wait for all agents to complete
    for pid in "${pids[@]}"; do
        wait "$pid"
    done
    
    echo "Parallel execution test completed"
}

# Test conflict detection
function test_conflict_detection() {
    local agent1="test-agent-1-$$"
    local agent2="test-agent-2-$$"
    
    echo "Testing conflict detection..."
    
    # Create conflicting scenario
    local task_file="/mnt/c/dev/gol/gol-bevy/tasks/setup-bevy-project.md"
    
    # Agent 1 acquires locks
    if acquire_file_locks "$agent1" "$task_file"; then
        echo "Agent 1 acquired locks successfully"
        
        # Agent 2 tries to acquire same locks
        if ! acquire_file_locks "$agent2" "$task_file"; then
            echo "Agent 2 correctly blocked from acquiring locks"
            echo "Conflict detection working properly"
        else
            echo "ERROR: Agent 2 should have been blocked"
        fi
        
        # Clean up
        release_agent_locks "$agent1"
    else
        echo "ERROR: Agent 1 failed to acquire locks"
    fi
}
```

### 7. Monitoring and Debugging

#### Coordination Status Dashboard
```bash
# Function to show coordination status
function show_coordination_status() {
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    
    echo "Coordination Status Dashboard"
    echo "============================"
    
    # Show active agents
    echo "Active Agents:"
    jq -r '.agents | to_entries[] | "  \(.key): \(.value.mode) - \(.value.current_task) (\(.value.status))"' \
        "$coordination_dir/active-agents.json" 2>/dev/null || echo "  No active agents"
    
    echo ""
    
    # Show active file locks
    echo "Active File Locks:"
    jq -r '.locks | to_entries[] | "  \(.key): locked by \(.value.locked_by) for \(.value.task)"' \
        "$coordination_dir/file-locks.json" 2>/dev/null || echo "  No active locks"
    
    echo ""
    
    # Show coordination health
    local coordination_enabled=$(jq -r '.coordination_enabled' "$coordination_dir/active-agents.json" 2>/dev/null)
    echo "Coordination Enabled: $coordination_enabled"
    
    # Show recent activity
    local last_updated=$(jq -r '.last_updated' "$coordination_dir/active-agents.json" 2>/dev/null)
    echo "Last Updated: $last_updated"
}
```

### 8. Performance Optimization

#### Coordination Overhead Monitoring
```bash
# Function to measure coordination overhead
function measure_coordination_overhead() {
    local task_file="$1"
    local agent_id="test-agent-$$"
    
    # Measure time without coordination
    local start_time=$(date +%s.%3N)
    # ... task execution without coordination ...
    local end_time=$(date +%s.%3N)
    local without_coordination=$(echo "$end_time - $start_time" | bc)
    
    # Measure time with coordination
    start_time=$(date +%s.%3N)
    validate_task_execution "$agent_id" "$task_file"
    acquire_file_locks "$agent_id" "$task_file"
    update_agent_status "$agent_id" "test" "$task_file" "active"
    # ... task execution with coordination ...
    release_agent_locks "$agent_id"
    remove_agent_status "$agent_id"
    end_time=$(date +%s.%3N)
    local with_coordination=$(echo "$end_time - $start_time" | bc)
    
    local overhead=$(echo "$with_coordination - $without_coordination" | bc)
    echo "Coordination overhead: ${overhead}s"
}
```

This integration guide provides comprehensive instructions for implementing coordination across all agent modes while maintaining the existing workflow patterns and ensuring safe parallel execution.