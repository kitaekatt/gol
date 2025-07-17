#!/bin/bash
# Lightweight Validation Procedures
# Implements fast, low-overhead validation checks for workflow integration

set -euo pipefail

# Configuration
VALIDATION_SCRIPT="/mnt/c/dev/gol/meta/memory/validate-memory.sh"
VALIDATION_LOG="/mnt/c/dev/gol/meta/memory/validation.log"
CACHE_DIR="/mnt/c/dev/gol/meta/memory/cache"
CACHE_DURATION=300  # 5 minutes

# Ensure cache directory exists
mkdir -p "$CACHE_DIR"

# Function to log validation events with minimal overhead
log_validation_event() {
    local level=$1
    local context=$2
    local message=$3
    echo "[$(date -Iseconds)] [$level] [$context] $message" >> "$VALIDATION_LOG"
}

# Function to get cached validation result
get_cached_validation() {
    local file_path=$1
    local cache_key=$(echo "$file_path" | sha256sum | cut -d' ' -f1)
    local cache_file="$CACHE_DIR/$cache_key"
    
    if [ -f "$cache_file" ]; then
        local cache_age=$(($(date +%s) - $(stat -c %Y "$cache_file")))
        if [ $cache_age -lt $CACHE_DURATION ]; then
            cat "$cache_file"
            return 0
        fi
    fi
    
    return 1
}

# Function to cache validation result
cache_validation_result() {
    local file_path=$1
    local result=$2
    local cache_key=$(echo "$file_path" | sha256sum | cut -d' ' -f1)
    local cache_file="$CACHE_DIR/$cache_key"
    
    echo "$result" > "$cache_file"
}

# Function to perform fast file validation with caching
fast_validate_file() {
    local file_path=$1
    local context=${2:-"general"}
    
    # Check cache first
    if cached_result=$(get_cached_validation "$file_path"); then
        log_validation_event "CACHE" "$context" "Cached validation result for $file_path: $cached_result"
        return $cached_result
    fi
    
    # Perform actual validation
    if "$VALIDATION_SCRIPT" validate "$file_path" >/dev/null 2>&1; then
        log_validation_event "PASS" "$context" "Validation passed: $file_path"
        cache_validation_result "$file_path" "0"
        return 0
    else
        log_validation_event "FAIL" "$context" "Validation failed: $file_path"
        cache_validation_result "$file_path" "1"
        return 1
    fi
}

# Function to validate multiple files in batch
batch_validate_files() {
    local context=$1
    shift
    local files=("$@")
    local failed_files=()
    
    log_validation_event "INFO" "$context" "Starting batch validation of ${#files[@]} files"
    
    for file in "${files[@]}"; do
        if ! fast_validate_file "$file" "$context"; then
            failed_files+=("$file")
        fi
    done
    
    if [ ${#failed_files[@]} -eq 0 ]; then
        log_validation_event "PASS" "$context" "Batch validation completed successfully"
        return 0
    else
        log_validation_event "FAIL" "$context" "Batch validation failed for: ${failed_files[*]}"
        return 1
    fi
}

# Pre-Mode Switch Validation
validate_pre_mode_switch() {
    local target_mode=$1
    local context="pre-mode-switch-$target_mode"
    
    log_validation_event "INFO" "$context" "Starting pre-mode switch validation"
    
    local core_files=(
        "/mnt/c/dev/gol/meta/docs/modes.md"
        "/mnt/c/dev/gol/CLAUDE.md"
        "/mnt/c/dev/gol/meta/docs/commands.md"
    )
    
    if batch_validate_files "$context" "${core_files[@]}"; then
        log_validation_event "PASS" "$context" "Pre-mode switch validation completed"
        return 0
    else
        log_validation_event "FAIL" "$context" "Pre-mode switch validation failed"
        return 1
    fi
}

# Post-Mode Switch Validation
validate_post_mode_switch() {
    local source_mode=$1
    local target_mode=$2
    local context="post-mode-switch-$source_mode-to-$target_mode"
    
    log_validation_event "INFO" "$context" "Starting post-mode switch validation"
    
    # Update baseline for mode switch
    "$VALIDATION_SCRIPT" post-check "/mnt/c/dev/gol/meta/docs/modes.md" "mode-switch" "Mode switch: $source_mode to $target_mode" >/dev/null 2>&1
    
    log_validation_event "PASS" "$context" "Post-mode switch validation completed"
    return 0
}

# Pre-Task Validation
validate_pre_task() {
    local mode=$1
    local task_name=${2:-"unknown"}
    local context="pre-task-$mode-$task_name"
    
    log_validation_event "INFO" "$context" "Starting pre-task validation"
    
    local core_files=(
        "/mnt/c/dev/gol/REQUIREMENTS.md"
        "/mnt/c/dev/gol/ARCHITECTURE.md"
        "/mnt/c/dev/gol/meta/docs/tasks.md"
    )
    
    # Add mode-specific validation
    case "$mode" in
        "bevy"|"entt"|"flecs"|"console")
            core_files+=("/mnt/c/dev/gol/meta/docs/modes.md")
            ;;
        "orchestrator")
            core_files+=("/mnt/c/dev/gol/meta/docs/modes.md")
            core_files+=("/mnt/c/dev/gol/PROJECTPLAN.md")
            ;;
        "meta")
            core_files+=("/mnt/c/dev/gol/meta/docs/development.md")
            core_files+=("/mnt/c/dev/gol/meta/docs/operations.md")
            ;;
    esac
    
    if batch_validate_files "$context" "${core_files[@]}"; then
        log_validation_event "PASS" "$context" "Pre-task validation completed"
        return 0
    else
        log_validation_event "FAIL" "$context" "Pre-task validation failed"
        return 1
    fi
}

# Post-Task Validation
validate_post_task() {
    local mode=$1
    local task_name=${2:-"unknown"}
    local modified_files=("${@:3}")
    local context="post-task-$mode-$task_name"
    
    log_validation_event "INFO" "$context" "Starting post-task validation"
    
    # Update baselines for modified files
    for file in "${modified_files[@]}"; do
        if [ -f "$file" ]; then
            "$VALIDATION_SCRIPT" post-check "$file" "$mode" "Task completion: $task_name" >/dev/null 2>&1
        fi
    done
    
    log_validation_event "PASS" "$context" "Post-task validation completed"
    return 0
}

# Pre-Orchestrator Communication Validation
validate_pre_orchestrator_comm() {
    local current_mode=$1
    local context="pre-orchestrator-comm-$current_mode"
    
    log_validation_event "INFO" "$context" "Starting pre-orchestrator communication validation"
    
    local core_files=(
        "/mnt/c/dev/gol/meta/docs/commands.md"
        "/mnt/c/dev/gol/meta/docs/modes.md"
    )
    
    if batch_validate_files "$context" "${core_files[@]}"; then
        log_validation_event "PASS" "$context" "Pre-orchestrator communication validation completed"
        return 0
    else
        log_validation_event "FAIL" "$context" "Pre-orchestrator communication validation failed"
        return 1
    fi
}

# Post-Orchestrator Communication Validation
validate_post_orchestrator_comm() {
    local current_mode=$1
    local context="post-orchestrator-comm-$current_mode"
    
    log_validation_event "INFO" "$context" "Starting post-orchestrator communication validation"
    
    # Update baseline for communication
    "$VALIDATION_SCRIPT" post-check "/mnt/c/dev/gol/meta/docs/modes.md" "orchestrator-communication" "Orchestrator communication from $current_mode" >/dev/null 2>&1
    
    log_validation_event "PASS" "$context" "Post-orchestrator communication validation completed"
    return 0
}

# Pre-Development Validation
validate_pre_development() {
    local mode=$1
    local context="pre-development-$mode"
    
    log_validation_event "INFO" "$context" "Starting pre-development validation"
    
    local core_files=(
        "/mnt/c/dev/gol/meta/docs/development.md"
        "/mnt/c/dev/gol/meta/docs/tasks.md"
    )
    
    if batch_validate_files "$context" "${core_files[@]}"; then
        log_validation_event "PASS" "$context" "Pre-development validation completed"
        return 0
    else
        log_validation_event "FAIL" "$context" "Pre-development validation failed"
        return 1
    fi
}

# Post-Development Validation
validate_post_development() {
    local mode=$1
    local modified_files=("${@:2}")
    local context="post-development-$mode"
    
    log_validation_event "INFO" "$context" "Starting post-development validation"
    
    # Update baselines for modified files
    for file in "${modified_files[@]}"; do
        if [ -f "$file" ]; then
            "$VALIDATION_SCRIPT" post-check "$file" "$mode" "Development completion" >/dev/null 2>&1
        fi
    done
    
    log_validation_event "PASS" "$context" "Post-development validation completed"
    return 0
}

# Pre-Maintenance Validation
validate_pre_maintenance() {
    local mode=$1
    local context="pre-maintenance-$mode"
    
    log_validation_event "INFO" "$context" "Starting pre-maintenance validation"
    
    # Use existing weekly validation
    if "$VALIDATION_SCRIPT" weekly >/dev/null 2>&1; then
        log_validation_event "PASS" "$context" "Pre-maintenance validation completed"
        return 0
    else
        log_validation_event "FAIL" "$context" "Pre-maintenance validation failed"
        return 1
    fi
}

# Post-Maintenance Validation
validate_post_maintenance() {
    local mode=$1
    local updated_files=("${@:2}")
    local context="post-maintenance-$mode"
    
    log_validation_event "INFO" "$context" "Starting post-maintenance validation"
    
    # Update baselines for updated files
    for file in "${updated_files[@]}"; do
        if [ -f "$file" ]; then
            "$VALIDATION_SCRIPT" post-check "$file" "$mode" "Maintenance completion" >/dev/null 2>&1
        fi
    done
    
    log_validation_event "PASS" "$context" "Post-maintenance validation completed"
    return 0
}

# Quick validation health check
validate_system_health() {
    local context="system-health"
    
    log_validation_event "INFO" "$context" "Starting system health validation"
    
    # Check if validation script is functional
    if ! "$VALIDATION_SCRIPT" status >/dev/null 2>&1; then
        log_validation_event "FAIL" "$context" "Validation script not functional"
        return 1
    fi
    
    # Check if cache directory is writable
    if ! touch "$CACHE_DIR/test" 2>/dev/null; then
        log_validation_event "FAIL" "$context" "Cache directory not writable"
        return 1
    fi
    rm -f "$CACHE_DIR/test"
    
    # Check if log file is writable
    if ! touch "$VALIDATION_LOG" 2>/dev/null; then
        log_validation_event "FAIL" "$context" "Log file not writable"
        return 1
    fi
    
    log_validation_event "PASS" "$context" "System health validation completed"
    return 0
}

# Clear validation cache
clear_validation_cache() {
    local context="cache-management"
    
    log_validation_event "INFO" "$context" "Clearing validation cache"
    
    rm -rf "$CACHE_DIR"/*
    
    log_validation_event "INFO" "$context" "Validation cache cleared"
}

# Show validation statistics
show_validation_stats() {
    local context="statistics"
    
    echo "=== Validation Statistics ==="
    echo
    
    if [ -f "$VALIDATION_LOG" ]; then
        echo "Recent validation events:"
        tail -10 "$VALIDATION_LOG"
        echo
        
        echo "Validation success rate (last 100 events):"
        local total_events=$(tail -100 "$VALIDATION_LOG" | wc -l)
        local pass_events=$(tail -100 "$VALIDATION_LOG" | grep -c "\[PASS\]" || echo "0")
        local fail_events=$(tail -100 "$VALIDATION_LOG" | grep -c "\[FAIL\]" || echo "0")
        
        if [ $total_events -gt 0 ]; then
            local success_rate=$(( (pass_events * 100) / total_events ))
            echo "  Total events: $total_events"
            echo "  Passed: $pass_events"
            echo "  Failed: $fail_events"
            echo "  Success rate: $success_rate%"
        else
            echo "  No validation events found"
        fi
    else
        echo "No validation log found"
    fi
    
    echo
    echo "Cache status:"
    if [ -d "$CACHE_DIR" ]; then
        local cache_files=$(find "$CACHE_DIR" -type f | wc -l)
        echo "  Cached validations: $cache_files"
        echo "  Cache directory size: $(du -sh "$CACHE_DIR" | cut -f1)"
    else
        echo "  No cache directory found"
    fi
}

# Main function to handle command-line arguments
main() {
    case "${1:-help}" in
        "pre-mode-switch")
            validate_pre_mode_switch "${2:-unknown}"
            ;;
        "post-mode-switch")
            validate_post_mode_switch "${2:-unknown}" "${3:-unknown}"
            ;;
        "pre-task")
            validate_pre_task "${2:-unknown}" "${3:-unknown}"
            ;;
        "post-task")
            validate_post_task "${2:-unknown}" "${3:-unknown}" "${@:4}"
            ;;
        "pre-orchestrator-comm")
            validate_pre_orchestrator_comm "${2:-unknown}"
            ;;
        "post-orchestrator-comm")
            validate_post_orchestrator_comm "${2:-unknown}"
            ;;
        "pre-development")
            validate_pre_development "${2:-unknown}"
            ;;
        "post-development")
            validate_post_development "${2:-unknown}" "${@:3}"
            ;;
        "pre-maintenance")
            validate_pre_maintenance "${2:-unknown}"
            ;;
        "post-maintenance")
            validate_post_maintenance "${2:-unknown}" "${@:3}"
            ;;
        "health")
            validate_system_health
            ;;
        "clear-cache")
            clear_validation_cache
            ;;
        "stats")
            show_validation_stats
            ;;
        "help")
            echo "Lightweight Validation Procedures"
            echo "Usage: $0 <command> [options]"
            echo
            echo "Commands:"
            echo "  pre-mode-switch <target_mode>                    - Pre-mode switch validation"
            echo "  post-mode-switch <source_mode> <target_mode>     - Post-mode switch validation"
            echo "  pre-task <mode> [task_name]                      - Pre-task validation"
            echo "  post-task <mode> [task_name] [modified_files...] - Post-task validation"
            echo "  pre-orchestrator-comm <current_mode>             - Pre-orchestrator communication validation"
            echo "  post-orchestrator-comm <current_mode>            - Post-orchestrator communication validation"
            echo "  pre-development <mode>                           - Pre-development validation"
            echo "  post-development <mode> [modified_files...]      - Post-development validation"
            echo "  pre-maintenance <mode>                           - Pre-maintenance validation"
            echo "  post-maintenance <mode> [updated_files...]       - Post-maintenance validation"
            echo "  health                                           - System health validation"
            echo "  clear-cache                                      - Clear validation cache"
            echo "  stats                                            - Show validation statistics"
            echo "  help                                             - Show this help message"
            ;;
        *)
            echo "Unknown command: $1"
            echo "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"