# Coordination Utilities

## Overview
This document provides utility functions and procedures for implementing the coordination protocol in agent workflows.

## Core Utility Functions

### 1. Task Validation
```bash
# Function to validate if a task can be executed
function validate_task_execution() {
    local agent_id="$1"
    local task_file="$2"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    
    # Check if coordination is enabled
    if ! jq -r '.coordination_enabled' "$coordination_dir/active-agents.json" | grep -q "true"; then
        echo "true" # Coordination disabled, allow execution
        return 0
    fi
    
    # Parse task metadata
    local task_metadata=$(extract_task_metadata "$task_file")
    
    # Check dependencies
    local depends_on=$(echo "$task_metadata" | jq -r '.depends_on[]?')
    for dependency in $depends_on; do
        if ! is_task_completed "$dependency"; then
            echo "false: Dependency not met: $dependency"
            return 1
        fi
    done
    
    # Check file conflicts
    local modifies_files=$(echo "$task_metadata" | jq -r '.modifies_files[]?')
    for file in $modifies_files; do
        if is_file_locked "$file" && ! is_lock_owned_by "$file" "$agent_id"; then
            echo "false: File locked by another agent: $file"
            return 1
        fi
    done
    
    # Check task conflicts
    local conflicts_with=$(echo "$task_metadata" | jq -r '.conflicts_with[]?')
    for conflict_task in $conflicts_with; do
        if is_task_active "$conflict_task"; then
            echo "false: Conflicting task active: $conflict_task"
            return 1
        fi
    done
    
    echo "true"
    return 0
}

# Function to extract task metadata from markdown file
function extract_task_metadata() {
    local task_file="$1"
    
    # Extract coordination metadata from markdown
    local classification=$(grep "^**Task Classification**:" "$task_file" | cut -d':' -f2 | xargs)
    local conflicts_with=$(grep "^**Conflicts With**:" "$task_file" | cut -d':' -f2 | xargs)
    local depends_on=$(grep "^**Depends On**:" "$task_file" | cut -d':' -f2 | xargs)
    local modifies_files=$(grep "^**Modifies Files**:" "$task_file" | cut -d':' -f2 | xargs)
    local reads_files=$(grep "^**Reads Files**:" "$task_file" | cut -d':' -f2 | xargs)
    local parallel_safe=$(grep "^**Parallel Safe**:" "$task_file" | cut -d':' -f2 | xargs)
    local estimated_duration=$(grep "^**Estimated Duration**:" "$task_file" | cut -d':' -f2 | xargs)
    local file_lock_priority=$(grep "^**File Lock Priority**:" "$task_file" | cut -d':' -f2 | xargs)
    
    # Convert to JSON
    jq -n \
        --arg classification "$classification" \
        --arg conflicts_with "$conflicts_with" \
        --arg depends_on "$depends_on" \
        --arg modifies_files "$modifies_files" \
        --arg reads_files "$reads_files" \
        --arg parallel_safe "$parallel_safe" \
        --arg estimated_duration "$estimated_duration" \
        --arg file_lock_priority "$file_lock_priority" \
        '{
            classification: $classification,
            conflicts_with: ($conflicts_with | split(",") | map(select(. != ""))),
            depends_on: ($depends_on | split(",") | map(select(. != ""))),
            modifies_files: ($modifies_files | split(",") | map(select(. != ""))),
            reads_files: ($reads_files | split(",") | map(select(. != ""))),
            parallel_safe: ($parallel_safe | test("Yes|yes|true"; "i")),
            estimated_duration: ($estimated_duration | tonumber? // 60),
            file_lock_priority: $file_lock_priority
        }'
}
```

### 2. Lock Management
```bash
# Function to acquire file locks
function acquire_file_locks() {
    local agent_id="$1"
    local task_file="$2"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    
    # Extract files to lock from task metadata
    local task_metadata=$(extract_task_metadata "$task_file")
    local modifies_files=$(echo "$task_metadata" | jq -r '.modifies_files[]?')
    local priority=$(echo "$task_metadata" | jq -r '.file_lock_priority')
    
    # Current timestamp
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local expires_at=$(date -u -d "+60 minutes" +"%Y-%m-%dT%H:%M:%SZ")
    
    # Acquire locks for each file
    for file in $modifies_files; do
        # Check if file is already locked
        local existing_lock=$(jq -r ".locks[\"$file\"]" "$locks_file")
        if [[ "$existing_lock" != "null" ]]; then
            local lock_owner=$(echo "$existing_lock" | jq -r '.locked_by')
            local lock_priority=$(echo "$existing_lock" | jq -r '.priority')
            
            if [[ "$lock_owner" != "$agent_id" ]]; then
                # Check if we can override based on priority
                if ! can_override_lock "$priority" "$lock_priority"; then
                    echo "false: Cannot acquire lock for $file (locked by $lock_owner)"
                    return 1
                fi
            fi
        fi
        
        # Acquire the lock
        local lock_data=$(jq -n \
            --arg locked_by "$agent_id" \
            --arg lock_type "write" \
            --arg locked_at "$current_time" \
            --arg expires_at "$expires_at" \
            --arg task "$task_file" \
            --arg priority "$priority" \
            '{
                locked_by: $locked_by,
                lock_type: $lock_type,
                locked_at: $locked_at,
                expires_at: $expires_at,
                task: $task,
                priority: $priority
            }')
        
        # Update locks file
        jq ".locks[\"$file\"] = $lock_data | .last_updated = \"$current_time\"" "$locks_file" > "$locks_file.tmp"
        mv "$locks_file.tmp" "$locks_file"
    done
    
    echo "true"
    return 0
}

# Function to release all locks held by an agent
function release_agent_locks() {
    local agent_id="$1"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    
    # Remove all locks held by this agent
    jq "del(.locks[] | select(.locked_by == \"$agent_id\")) | .last_updated = \"$current_time\"" "$locks_file" > "$locks_file.tmp"
    mv "$locks_file.tmp" "$locks_file"
}
```

### 3. Agent Status Management
```bash
# Function to update agent status
function update_agent_status() {
    local agent_id="$1"
    local mode="$2"
    local current_task="$3"
    local status="$4"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local agents_file="$coordination_dir/active-agents.json"
    
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    
    # Calculate estimated completion based on task metadata
    local estimated_completion="$current_time"
    if [[ -n "$current_task" ]]; then
        local task_metadata=$(extract_task_metadata "$current_task")
        local duration=$(echo "$task_metadata" | jq -r '.estimated_duration')
        estimated_completion=$(date -u -d "+${duration} minutes" +"%Y-%m-%dT%H:%M:%SZ")
    fi
    
    # Get locked files for this agent
    local locked_files=$(jq -r ".locks | to_entries[] | select(.value.locked_by == \"$agent_id\") | .key" "$coordination_dir/file-locks.json" | jq -Rs 'split("\n") | map(select(. != ""))')
    
    # Update agent status
    local agent_data=$(jq -n \
        --arg mode "$mode" \
        --arg current_task "$current_task" \
        --arg task_started "$current_time" \
        --arg estimated_completion "$estimated_completion" \
        --argjson locked_files "$locked_files" \
        --arg status "$status" \
        --arg parallel_compatible "true" \
        --arg priority "medium" \
        '{
            mode: $mode,
            current_task: $current_task,
            task_started: $task_started,
            estimated_completion: $estimated_completion,
            locked_files: $locked_files,
            status: $status,
            parallel_compatible: ($parallel_compatible | test("true"; "i")),
            priority: $priority
        }')
    
    # Update agents file
    jq ".agents[\"$agent_id\"] = $agent_data | .last_updated = \"$current_time\"" "$agents_file" > "$agents_file.tmp"
    mv "$agents_file.tmp" "$agents_file"
}

# Function to remove agent from active status
function remove_agent_status() {
    local agent_id="$1"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local agents_file="$coordination_dir/active-agents.json"
    
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    
    # Remove agent from active agents
    jq "del(.agents[\"$agent_id\"]) | .last_updated = \"$current_time\"" "$agents_file" > "$agents_file.tmp"
    mv "$agents_file.tmp" "$agents_file"
}
```

### 4. Conflict Detection
```bash
# Function to check if a task is currently active
function is_task_active() {
    local task_name="$1"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local agents_file="$coordination_dir/active-agents.json"
    
    local active_tasks=$(jq -r '.agents[] | select(.status == "active") | .current_task' "$agents_file")
    echo "$active_tasks" | grep -q "^$task_name$"
}

# Function to check if a file is locked
function is_file_locked() {
    local file_path="$1"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    
    local lock=$(jq -r ".locks[\"$file_path\"]" "$locks_file")
    [[ "$lock" != "null" ]]
}

# Function to check if a lock is owned by a specific agent
function is_lock_owned_by() {
    local file_path="$1"
    local agent_id="$2"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    
    local lock_owner=$(jq -r ".locks[\"$file_path\"].locked_by" "$locks_file")
    [[ "$lock_owner" == "$agent_id" ]]
}

# Function to check if a task is completed
function is_task_completed() {
    local task_name="$1"
    
    # Check if task exists in any completed directory
    find /mnt/c/dev/gol -path "*/tasks/completed/$task_name" -type f | grep -q .
}
```

### 5. Cleanup and Maintenance
```bash
# Function to clean up expired locks
function cleanup_expired_locks() {
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    
    # Remove expired locks
    jq "
        .locks = (.locks | with_entries(
            select(.value.expires_at > \"$current_time\")
        )) |
        .last_updated = \"$current_time\"
    " "$locks_file" > "$locks_file.tmp"
    mv "$locks_file.tmp" "$locks_file"
}

# Function to detect and clean up stale agent status
function cleanup_stale_agents() {
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local agents_file="$coordination_dir/active-agents.json"
    local current_time=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local stale_threshold=$(date -u -d "-120 minutes" +"%Y-%m-%dT%H:%M:%SZ")
    
    # Remove agents that haven't updated in 2 hours
    jq "
        .agents = (.agents | with_entries(
            select(.value.task_started > \"$stale_threshold\")
        )) |
        .last_updated = \"$current_time\"
    " "$agents_file" > "$agents_file.tmp"
    mv "$agents_file.tmp" "$agents_file"
}

# Function to perform full coordination cleanup
function coordination_cleanup() {
    cleanup_expired_locks
    cleanup_stale_agents
}
```

## Integration with Agent Workflows

### Agent Mode Template Integration
Add to each agent mode template:

```bash
# Coordination Protocol Integration
COORDINATION_DIR="/mnt/c/dev/gol/meta/coordination"
AGENT_ID="$(uname -n)-$(date +%s)-$$"  # Unique agent identifier

# Before task execution
if ! validate_task_execution "$AGENT_ID" "$TASK_FILE"; then
    echo "Task cannot be executed due to coordination constraints"
    exit 1
fi

# Acquire necessary locks
if ! acquire_file_locks "$AGENT_ID" "$TASK_FILE"; then
    echo "Failed to acquire necessary file locks"
    exit 1
fi

# Update agent status
update_agent_status "$AGENT_ID" "$MODE" "$TASK_FILE" "active"

# Execute task
trap 'release_agent_locks "$AGENT_ID"; remove_agent_status "$AGENT_ID"' EXIT

# ... task execution code ...

# On completion
update_agent_status "$AGENT_ID" "$MODE" "$TASK_FILE" "completed"
```

### Usage Examples

```bash
# Check if a task can be executed
if validate_task_execution "agent-123" "/path/to/task.md"; then
    echo "Task can be executed"
fi

# Acquire locks for a task
if acquire_file_locks "agent-123" "/path/to/task.md"; then
    echo "Locks acquired successfully"
fi

# Update agent status
update_agent_status "agent-123" "bevy" "setup-project.md" "active"

# Clean up on completion
release_agent_locks "agent-123"
remove_agent_status "agent-123"
```

## Error Handling Procedures

### Lock Acquisition Failures
```bash
function handle_lock_failure() {
    local agent_id="$1"
    local task_file="$2"
    
    echo "Failed to acquire locks for $task_file"
    
    # Try to find alternative tasks
    local alternative_tasks=$(find_parallel_safe_tasks "$agent_id")
    if [[ -n "$alternative_tasks" ]]; then
        echo "Alternative tasks available: $alternative_tasks"
    else
        echo "No alternative tasks available, entering wait state"
    fi
}
```

### Agent Failure Recovery
```bash
function recover_from_agent_failure() {
    local failed_agent_id="$1"
    
    # Release all locks held by failed agent
    release_agent_locks "$failed_agent_id"
    
    # Remove from active agents
    remove_agent_status "$failed_agent_id"
    
    # Notify orchestrator of failure
    echo "Agent $failed_agent_id failed, coordination state cleaned up"
}
```

## Performance Optimization

### Batch Operations
```bash
# Batch lock acquisition for multiple files
function batch_acquire_locks() {
    local agent_id="$1"
    local files=("${@:2}")
    
    # Acquire all locks atomically
    local temp_locks=()
    for file in "${files[@]}"; do
        if ! acquire_single_lock "$agent_id" "$file"; then
            # Roll back all acquired locks
            for acquired_lock in "${temp_locks[@]}"; do
                release_single_lock "$agent_id" "$acquired_lock"
            done
            return 1
        fi
        temp_locks+=("$file")
    done
    
    return 0
}
```

### Caching and Optimization
```bash
# Cache task metadata to avoid repeated parsing
declare -A TASK_METADATA_CACHE

function get_cached_task_metadata() {
    local task_file="$1"
    local cache_key="$task_file"
    
    if [[ -z "${TASK_METADATA_CACHE[$cache_key]}" ]]; then
        TASK_METADATA_CACHE[$cache_key]=$(extract_task_metadata "$task_file")
    fi
    
    echo "${TASK_METADATA_CACHE[$cache_key]}"
}
```

This utilities framework provides the foundation for implementing coordination across all agent modes while maintaining the lightweight, file-based approach specified in the requirements.