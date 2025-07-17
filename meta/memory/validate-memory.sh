#!/bin/bash
# Memory Validation Script
# Implements validation checkpoints to prevent documentation drift

set -euo pipefail

BASELINE_DIR="/mnt/c/dev/gol/meta/memory/baselines"
CURRENT_BASELINE="$BASELINE_DIR/current-baseline.json"
VALIDATION_LOG="/mnt/c/dev/gol/meta/memory/validation.log"

# Function to log validation events
log_validation() {
    local level=$1
    local message=$2
    echo "[$(date -Iseconds)] [$level] $message" >> "$VALIDATION_LOG"
    echo "[$level] $message"
}

# Function to validate a single file against baseline
validate_file() {
    local file_path=$1
    local baseline_file=${2:-$CURRENT_BASELINE}
    
    if [ ! -f "$file_path" ]; then
        log_validation "ERROR" "File not found: $file_path"
        return 1
    fi
    
    if [ ! -f "$baseline_file" ]; then
        log_validation "ERROR" "Baseline file not found: $baseline_file"
        return 1
    fi
    
    # Generate current hash
    local current_hash=$(sha256sum "$file_path" 2>/dev/null | cut -d' ' -f1)
    
    # Get baseline hash
    local baseline_hash=$(jq -r ".files[\"$file_path\"].hash" "$baseline_file" 2>/dev/null)
    
    if [ "$baseline_hash" = "null" ]; then
        log_validation "WARN" "No baseline found for: $file_path"
        return 2
    fi
    
    if [ "$current_hash" != "$baseline_hash" ]; then
        log_validation "DRIFT" "Documentation drift detected in: $file_path"
        log_validation "DRIFT" "  Current:  $current_hash"
        log_validation "DRIFT" "  Baseline: $baseline_hash"
        return 1
    fi
    
    log_validation "OK" "Validation passed: $file_path"
    return 0
}

# Function to generate baseline for a file
generate_baseline() {
    local file_path=$1
    local validator=${2:-"meta-mode"}
    local notes=${3:-"Baseline update"}
    
    if [ ! -f "$file_path" ]; then
        log_validation "ERROR" "Cannot generate baseline for missing file: $file_path"
        return 1
    fi
    
    local current_hash=$(sha256sum "$file_path" | cut -d' ' -f1)
    local current_size=$(stat -c%s "$file_path")
    local current_time=$(date -Iseconds)
    
    # Create baseline file if it doesn't exist
    if [ ! -f "$CURRENT_BASELINE" ]; then
        echo '{"timestamp":"","baseline_type":"current","validator":"","files":{},"validation_protocol":"","next_validation":""}' > "$CURRENT_BASELINE"
    fi
    
    # Update baseline file
    jq --arg path "$file_path" \
       --arg hash "$current_hash" \
       --arg size "$current_size" \
       --arg time "$current_time" \
       --arg validator "$validator" \
       --arg notes "$notes" \
       --arg baseline_time "$(date -Iseconds)" \
       '.timestamp = $baseline_time |
        .files[$path] = {
            "hash": $hash,
            "size": ($size | tonumber),
            "last_modified": $time,
            "validator": $validator,
            "notes": $notes
        }' "$CURRENT_BASELINE" > "$CURRENT_BASELINE.tmp" && \
    mv "$CURRENT_BASELINE.tmp" "$CURRENT_BASELINE"
    
    log_validation "UPDATE" "Baseline updated for: $file_path"
    return 0
}

# Function to perform pre-modification checkpoint
pre_modification_checkpoint() {
    local file_path=$1
    
    log_validation "INFO" "Starting pre-modification checkpoint for: $file_path"
    
    if validate_file "$file_path"; then
        log_validation "INFO" "Pre-modification checkpoint passed: $file_path"
        return 0
    else
        local exit_code=$?
        if [ $exit_code -eq 2 ]; then
            log_validation "WARN" "No baseline exists, creating one for: $file_path"
            generate_baseline "$file_path" "meta-mode" "Initial baseline creation"
            return 0
        else
            log_validation "ERROR" "Pre-modification checkpoint failed: $file_path"
            return 1
        fi
    fi
}

# Function to perform post-modification checkpoint
post_modification_checkpoint() {
    local file_path=$1
    local validator=${2:-"meta-mode"}
    local notes=${3:-"Post-modification update"}
    
    log_validation "INFO" "Starting post-modification checkpoint for: $file_path"
    
    if generate_baseline "$file_path" "$validator" "$notes"; then
        log_validation "INFO" "Post-modification checkpoint completed: $file_path"
        return 0
    else
        log_validation "ERROR" "Post-modification checkpoint failed: $file_path"
        return 1
    fi
}

# Function to validate all core documentation files
validate_all_core_files() {
    local exit_code=0
    local core_files=(
        "/mnt/c/dev/gol/CLAUDE.md"
        "/mnt/c/dev/gol/ARCHITECTURE.md"
        "/mnt/c/dev/gol/REQUIREMENTS.md"
        "/mnt/c/dev/gol/PROJECTPLAN.md"
        "/mnt/c/dev/gol/meta/docs/commands.md"
        "/mnt/c/dev/gol/meta/docs/modes.md"
        "/mnt/c/dev/gol/meta/docs/tasks.md"
        "/mnt/c/dev/gol/meta/docs/development.md"
        "/mnt/c/dev/gol/meta/docs/architecture.md"
        "/mnt/c/dev/gol/meta/docs/operations.md"
    )
    
    log_validation "INFO" "Starting validation of all core documentation files"
    
    for file in "${core_files[@]}"; do
        if [ -f "$file" ]; then
            if ! validate_file "$file"; then
                exit_code=1
            fi
        else
            log_validation "WARN" "Core file not found: $file"
        fi
    done
    
    if [ $exit_code -eq 0 ]; then
        log_validation "INFO" "All core documentation validation completed successfully"
    else
        log_validation "ERROR" "Some core documentation validation failures detected"
    fi
    
    return $exit_code
}

# Function to perform weekly maintenance validation
weekly_maintenance_validation() {
    log_validation "INFO" "Starting weekly maintenance validation"
    
    # Validate all core files
    validate_all_core_files
    local core_exit_code=$?
    
    # Generate report summary
    local total_files=$(jq '.files | length' "$CURRENT_BASELINE" 2>/dev/null || echo "0")
    local last_update=$(jq -r '.timestamp' "$CURRENT_BASELINE" 2>/dev/null || echo "unknown")
    
    log_validation "INFO" "Weekly maintenance summary:"
    log_validation "INFO" "  Total files in baseline: $total_files"
    log_validation "INFO" "  Last baseline update: $last_update"
    log_validation "INFO" "  Core validation result: $([ $core_exit_code -eq 0 ] && echo "PASS" || echo "FAIL")"
    
    return $core_exit_code
}

# Function to show validation status
show_validation_status() {
    echo "=== Memory Validation Status ==="
    echo
    
    if [ -f "$CURRENT_BASELINE" ]; then
        local total_files=$(jq '.files | length' "$CURRENT_BASELINE")
        local last_update=$(jq -r '.timestamp' "$CURRENT_BASELINE")
        echo "Baseline Status: Active"
        echo "Total Files: $total_files"
        echo "Last Update: $last_update"
        echo
        
        echo "Files in baseline:"
        jq -r '.files | keys[]' "$CURRENT_BASELINE" | while read -r file; do
            echo "  - $file"
        done
    else
        echo "Baseline Status: Not found"
        echo "Run 'generate_baseline' to create initial baseline"
    fi
    
    echo
    if [ -f "$VALIDATION_LOG" ]; then
        echo "Recent validation events:"
        tail -5 "$VALIDATION_LOG"
    else
        echo "No validation log found"
    fi
}

# Main function to handle command-line arguments
main() {
    case "${1:-help}" in
        "validate")
            if [ $# -eq 2 ]; then
                validate_file "$2"
            else
                validate_all_core_files
            fi
            ;;
        "pre-check")
            if [ $# -eq 2 ]; then
                pre_modification_checkpoint "$2"
            else
                echo "Usage: $0 pre-check <file_path>"
                exit 1
            fi
            ;;
        "post-check")
            if [ $# -ge 2 ]; then
                post_modification_checkpoint "$2" "${3:-meta-mode}" "${4:-Post-modification update}"
            else
                echo "Usage: $0 post-check <file_path> [validator] [notes]"
                exit 1
            fi
            ;;
        "generate")
            if [ $# -ge 2 ]; then
                generate_baseline "$2" "${3:-meta-mode}" "${4:-Manual baseline generation}"
            else
                echo "Usage: $0 generate <file_path> [validator] [notes]"
                exit 1
            fi
            ;;
        "weekly")
            weekly_maintenance_validation
            ;;
        "status")
            show_validation_status
            ;;
        "help")
            echo "Memory Validation Script"
            echo "Usage: $0 <command> [options]"
            echo
            echo "Commands:"
            echo "  validate [file]    - Validate file(s) against baseline"
            echo "  pre-check <file>   - Pre-modification validation checkpoint"
            echo "  post-check <file>  - Post-modification validation checkpoint"
            echo "  generate <file>    - Generate baseline for file"
            echo "  weekly            - Weekly maintenance validation"
            echo "  status            - Show validation status"
            echo "  help              - Show this help message"
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