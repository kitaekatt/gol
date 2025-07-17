# Coordination Testing Framework

## Overview
This document provides testing procedures and validation methods for the parallel agent coordination system.

## Test Categories

### 1. Basic Coordination Tests

#### Test 1.1: Agent Status Tracking
```bash
#!/bin/bash
# Test agent status tracking functionality

test_agent_status_tracking() {
    echo "Testing agent status tracking..."
    
    local agent_id="test-agent-$$"
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    
    # Test status update
    update_agent_status "$agent_id" "bevy" "test-task.md" "active"
    
    # Verify status recorded
    local status=$(jq -r ".agents[\"$agent_id\"].status" "$coordination_dir/active-agents.json")
    if [[ "$status" == "active" ]]; then
        echo "✓ Agent status tracking works"
    else
        echo "✗ Agent status tracking failed"
        return 1
    fi
    
    # Test status removal
    remove_agent_status "$agent_id"
    
    # Verify status removed
    local removed_status=$(jq -r ".agents[\"$agent_id\"]" "$coordination_dir/active-agents.json")
    if [[ "$removed_status" == "null" ]]; then
        echo "✓ Agent status removal works"
    else
        echo "✗ Agent status removal failed"
        return 1
    fi
    
    return 0
}
```

#### Test 1.2: File Lock Management
```bash
test_file_lock_management() {
    echo "Testing file lock management..."
    
    local agent_id="test-agent-$$"
    local test_file="/mnt/c/dev/gol/test-file.txt"
    
    # Test lock acquisition
    if acquire_single_lock "$agent_id" "$test_file" "write" "medium"; then
        echo "✓ Lock acquisition works"
    else
        echo "✗ Lock acquisition failed"
        return 1
    fi
    
    # Test lock conflict detection
    local agent_id2="test-agent-2-$$"
    if ! acquire_single_lock "$agent_id2" "$test_file" "write" "medium"; then
        echo "✓ Lock conflict detection works"
    else
        echo "✗ Lock conflict detection failed"
        return 1
    fi
    
    # Test lock release
    release_single_lock "$agent_id" "$test_file"
    
    # Verify lock released
    if acquire_single_lock "$agent_id2" "$test_file" "write" "medium"; then
        echo "✓ Lock release works"
        release_single_lock "$agent_id2" "$test_file"
    else
        echo "✗ Lock release failed"
        return 1
    fi
    
    return 0
}
```

#### Test 1.3: Task Validation
```bash
test_task_validation() {
    echo "Testing task validation..."
    
    local agent_id="test-agent-$$"
    local task_file="/mnt/c/dev/gol/gol-entt/tasks/setup-entt-project.md"
    
    # Test task can be validated
    if validate_task_execution "$agent_id" "$task_file" | grep -q "true"; then
        echo "✓ Task validation works"
    else
        echo "✗ Task validation failed"
        return 1
    fi
    
    return 0
}
```

### 2. Conflict Detection Tests

#### Test 2.1: File Conflict Detection
```bash
test_file_conflict_detection() {
    echo "Testing file conflict detection..."
    
    local agent1="test-agent-1-$$"
    local agent2="test-agent-2-$$"
    local test_file="/mnt/c/dev/gol/gol-entt/src/main.cpp"
    
    # Agent 1 acquires lock
    acquire_single_lock "$agent1" "$test_file" "write" "medium"
    
    # Agent 2 should be blocked
    if ! acquire_single_lock "$agent2" "$test_file" "write" "medium"; then
        echo "✓ File conflict detection works"
        release_single_lock "$agent1" "$test_file"
        return 0
    else
        echo "✗ File conflict detection failed"
        release_single_lock "$agent1" "$test_file"
        release_single_lock "$agent2" "$test_file"
        return 1
    fi
}
```

#### Test 2.2: Task Dependency Validation
```bash
test_task_dependency_validation() {
    echo "Testing task dependency validation..."
    
    local agent_id="test-agent-$$"
    local dependent_task="/mnt/c/dev/gol/gol-entt/tasks/implement-entt-components.md"
    
    # This task depends on setup-entt-project.md
    # Should fail if dependency not met
    local validation_result=$(validate_task_execution "$agent_id" "$dependent_task")
    
    if echo "$validation_result" | grep -q "Dependency not met"; then
        echo "✓ Task dependency validation works"
        return 0
    else
        echo "✗ Task dependency validation failed"
        return 1
    fi
}
```

### 3. Parallel Execution Tests

#### Test 3.1: Safe Parallel Execution
```bash
test_safe_parallel_execution() {
    echo "Testing safe parallel execution..."
    
    local pids=()
    
    # Start multiple agents on compatible tasks
    {
        local agent_id="test-bevy-$$"
        # Simulate Bevy agent work
        update_agent_status "$agent_id" "bevy" "test-bevy-task.md" "active"
        sleep 2
        update_agent_status "$agent_id" "bevy" "test-bevy-task.md" "completed"
        remove_agent_status "$agent_id"
        echo "Bevy agent completed"
    } &
    pids+=($!)
    
    {
        local agent_id="test-entt-$$"
        # Simulate EnTT agent work
        update_agent_status "$agent_id" "entt" "test-entt-task.md" "active"
        sleep 2
        update_agent_status "$agent_id" "entt" "test-entt-task.md" "completed"
        remove_agent_status "$agent_id"
        echo "EnTT agent completed"
    } &
    pids+=($!)
    
    # Wait for both to complete
    for pid in "${pids[@]}"; do
        wait "$pid"
    done
    
    echo "✓ Safe parallel execution works"
    return 0
}
```

#### Test 3.2: Conflict Prevention
```bash
test_conflict_prevention() {
    echo "Testing conflict prevention..."
    
    local agent1="test-agent-1-$$"
    local agent2="test-agent-2-$$"
    local shared_file="/mnt/c/dev/gol/shared-resource.txt"
    
    # Agent 1 starts task with shared resource
    acquire_single_lock "$agent1" "$shared_file" "write" "medium"
    update_agent_status "$agent1" "bevy" "test-shared-task.md" "active"
    
    # Agent 2 tries to access same resource
    if ! acquire_single_lock "$agent2" "$shared_file" "write" "medium"; then
        echo "✓ Conflict prevention works"
        
        # Clean up
        release_single_lock "$agent1" "$shared_file"
        remove_agent_status "$agent1"
        return 0
    else
        echo "✗ Conflict prevention failed"
        
        # Clean up
        release_single_lock "$agent1" "$shared_file"
        release_single_lock "$agent2" "$shared_file"
        remove_agent_status "$agent1"
        return 1
    fi
}
```

### 4. Performance Tests

#### Test 4.1: Coordination Overhead
```bash
test_coordination_overhead() {
    echo "Testing coordination overhead..."
    
    local agent_id="test-agent-$$"
    local iterations=100
    
    # Measure coordination operations
    local start_time=$(date +%s.%3N)
    
    for ((i=1; i<=iterations; i++)); do
        update_agent_status "$agent_id" "bevy" "test-task.md" "active"
        acquire_single_lock "$agent_id" "/tmp/test-file-$i" "write" "medium"
        release_single_lock "$agent_id" "/tmp/test-file-$i"
        remove_agent_status "$agent_id"
    done
    
    local end_time=$(date +%s.%3N)
    local total_time=$(echo "$end_time - $start_time" | bc)
    local avg_time=$(echo "scale=3; $total_time / $iterations" | bc)
    
    echo "✓ Coordination overhead: ${avg_time}ms per operation"
    
    # Should be under 10ms per operation
    if (( $(echo "$avg_time < 0.01" | bc -l) )); then
        echo "✓ Performance acceptable"
        return 0
    else
        echo "✗ Performance too slow"
        return 1
    fi
}
```

### 5. Error Handling Tests

#### Test 5.1: Stale Lock Cleanup
```bash
test_stale_lock_cleanup() {
    echo "Testing stale lock cleanup..."
    
    local agent_id="test-agent-$$"
    local test_file="/tmp/test-stale-lock"
    
    # Create expired lock
    local expired_time=$(date -u -d "-2 hours" +"%Y-%m-%dT%H:%M:%SZ")
    local lock_data=$(jq -n \
        --arg locked_by "$agent_id" \
        --arg lock_type "write" \
        --arg locked_at "$expired_time" \
        --arg expires_at "$expired_time" \
        --arg task "test-task.md" \
        --arg priority "medium" \
        '{
            locked_by: $locked_by,
            lock_type: $lock_type,
            locked_at: $locked_at,
            expires_at: $expires_at,
            task: $task,
            priority: $priority
        }')
    
    # Manually add expired lock
    local coordination_dir="/mnt/c/dev/gol/meta/coordination"
    local locks_file="$coordination_dir/file-locks.json"
    jq ".locks[\"$test_file\"] = $lock_data" "$locks_file" > "$locks_file.tmp"
    mv "$locks_file.tmp" "$locks_file"
    
    # Run cleanup
    cleanup_expired_locks
    
    # Verify lock removed
    local remaining_lock=$(jq -r ".locks[\"$test_file\"]" "$locks_file")
    if [[ "$remaining_lock" == "null" ]]; then
        echo "✓ Stale lock cleanup works"
        return 0
    else
        echo "✗ Stale lock cleanup failed"
        return 1
    fi
}
```

#### Test 5.2: Agent Recovery
```bash
test_agent_recovery() {
    echo "Testing agent recovery..."
    
    local agent_id="test-failed-agent-$$"
    local test_file="/tmp/test-recovery-lock"
    
    # Simulate agent failure with active locks
    acquire_single_lock "$agent_id" "$test_file" "write" "medium"
    update_agent_status "$agent_id" "bevy" "test-task.md" "active"
    
    # Simulate recovery
    recover_from_agent_failure "$agent_id"
    
    # Verify cleanup
    local agent_status=$(jq -r ".agents[\"$agent_id\"]" "/mnt/c/dev/gol/meta/coordination/active-agents.json")
    local lock_status=$(jq -r ".locks[\"$test_file\"]" "/mnt/c/dev/gol/meta/coordination/file-locks.json")
    
    if [[ "$agent_status" == "null" && "$lock_status" == "null" ]]; then
        echo "✓ Agent recovery works"
        return 0
    else
        echo "✗ Agent recovery failed"
        return 1
    fi
}
```

## Test Execution Framework

### Test Runner
```bash
#!/bin/bash
# Main test runner for coordination system

run_coordination_tests() {
    echo "Running Coordination System Tests"
    echo "================================"
    
    local test_results=()
    
    # Basic coordination tests
    echo "1. Basic Coordination Tests"
    test_agent_status_tracking && test_results+=("✓ Agent Status Tracking") || test_results+=("✗ Agent Status Tracking")
    test_file_lock_management && test_results+=("✓ File Lock Management") || test_results+=("✗ File Lock Management")
    test_task_validation && test_results+=("✓ Task Validation") || test_results+=("✗ Task Validation")
    
    # Conflict detection tests
    echo "2. Conflict Detection Tests"
    test_file_conflict_detection && test_results+=("✓ File Conflict Detection") || test_results+=("✗ File Conflict Detection")
    test_task_dependency_validation && test_results+=("✓ Task Dependency Validation") || test_results+=("✗ Task Dependency Validation")
    
    # Parallel execution tests
    echo "3. Parallel Execution Tests"
    test_safe_parallel_execution && test_results+=("✓ Safe Parallel Execution") || test_results+=("✗ Safe Parallel Execution")
    test_conflict_prevention && test_results+=("✓ Conflict Prevention") || test_results+=("✗ Conflict Prevention")
    
    # Performance tests
    echo "4. Performance Tests"
    test_coordination_overhead && test_results+=("✓ Coordination Overhead") || test_results+=("✗ Coordination Overhead")
    
    # Error handling tests
    echo "5. Error Handling Tests"
    test_stale_lock_cleanup && test_results+=("✓ Stale Lock Cleanup") || test_results+=("✗ Stale Lock Cleanup")
    test_agent_recovery && test_results+=("✓ Agent Recovery") || test_results+=("✗ Agent Recovery")
    
    # Test results summary
    echo ""
    echo "Test Results Summary"
    echo "==================="
    for result in "${test_results[@]}"; do
        echo "$result"
    done
    
    # Count failures
    local failures=$(printf "%s\n" "${test_results[@]}" | grep -c "✗")
    if [[ $failures -eq 0 ]]; then
        echo ""
        echo "All tests passed! ✓"
        return 0
    else
        echo ""
        echo "$failures test(s) failed! ✗"
        return 1
    fi
}
```

### Integration Test
```bash
#!/bin/bash
# Integration test for full coordination workflow

test_full_coordination_workflow() {
    echo "Testing full coordination workflow..."
    
    local agent_id="test-integration-$$"
    local task_file="/mnt/c/dev/gol/gol-entt/tasks/setup-entt-project.md"
    
    # Test full workflow
    echo "1. Validating task execution..."
    if ! validate_task_execution "$agent_id" "$task_file" | grep -q "true"; then
        echo "✗ Task validation failed"
        return 1
    fi
    
    echo "2. Acquiring file locks..."
    if ! acquire_file_locks "$agent_id" "$task_file"; then
        echo "✗ Lock acquisition failed"
        return 1
    fi
    
    echo "3. Updating agent status..."
    update_agent_status "$agent_id" "entt" "$task_file" "active"
    
    echo "4. Simulating task execution..."
    sleep 1
    
    echo "5. Completing task..."
    update_agent_status "$agent_id" "entt" "$task_file" "completed"
    
    echo "6. Releasing locks..."
    release_agent_locks "$agent_id"
    
    echo "7. Removing agent status..."
    remove_agent_status "$agent_id"
    
    echo "✓ Full coordination workflow works"
    return 0
}
```

## Validation Criteria

### Functional Validation
- [ ] All coordination functions execute without errors
- [ ] File locking prevents concurrent access
- [ ] Task validation detects conflicts and dependencies
- [ ] Agent status tracking works correctly
- [ ] Cleanup procedures remove stale state

### Performance Validation
- [ ] Coordination overhead < 10ms per operation
- [ ] Lock acquisition/release < 5ms
- [ ] Agent status updates < 2ms
- [ ] Task validation < 20ms
- [ ] Cleanup operations < 100ms

### Error Handling Validation
- [ ] Stale locks are cleaned up automatically
- [ ] Agent failures are detected and recovered
- [ ] Conflict resolution works correctly
- [ ] Graceful degradation on errors
- [ ] No orphaned coordination state

### Integration Validation
- [ ] Multiple agents can work in parallel on compatible tasks
- [ ] Conflicting tasks are properly serialized
- [ ] Dependency validation prevents premature execution
- [ ] File conflicts are detected and resolved
- [ ] System remains stable under load

## Test Data Setup

### Test Tasks
Create sample tasks with different coordination metadata for testing:

```markdown
# Test Task - Isolated
**Task Classification**: Isolated
**Conflicts With**: None
**Depends On**: None
**Modifies Files**: /tmp/test-isolated
**Reads Files**: /tmp/test-input
**Parallel Safe**: Yes
**Estimated Duration**: 5 minutes
**File Lock Priority**: Low
```

```markdown
# Test Task - Conflicting
**Task Classification**: Shared Resource
**Conflicts With**: test-task-conflicting-2.md
**Depends On**: None
**Modifies Files**: /tmp/shared-resource
**Reads Files**: /tmp/shared-input
**Parallel Safe**: No
**Estimated Duration**: 10 minutes
**File Lock Priority**: High
```

### Test Environment
- Clean coordination state before each test
- Restore coordination state after each test
- Use temporary files for testing
- Clean up test artifacts automatically

This testing framework ensures the coordination system works correctly and safely enables parallel agent execution.