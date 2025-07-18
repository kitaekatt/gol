#!/bin/bash

# GitHub API Core Operations for AI Task Management
# Provides CRUD operations for managing tasks via GitHub Issues
# Designed for AI agent consumption with structured, parseable output

set -euo pipefail

# Configuration
REPO_OWNER="kitaekatt"
REPO_NAME="gol"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Status label mappings for AI task management
declare -A STATUS_LABELS=(
    ["todo"]="status:todo"
    ["in-progress"]="status:in-progress" 
    ["done"]="status:done"
    ["blocked"]="status:blocked"
)

declare -A PRIORITY_LABELS=(
    ["high"]="priority:high"
    ["medium"]="priority:medium"
    ["low"]="priority:low"
)

declare -A AGENT_LABELS=(
    ["bevy"]="agent:bevy"
    ["entt"]="agent:entt"
    ["flecs"]="agent:flecs"
    ["console"]="agent:console"
    ["meta"]="agent:meta"
    ["orchestrator"]="agent:orchestrator"
)

# Utility functions
log_info() { echo -e "\033[34mℹ️  $1\033[0m" >&2; }
log_success() { echo -e "\033[32m✅ $1\033[0m" >&2; }
log_warning() { echo -e "\033[33m⚠️  $1\033[0m" >&2; }
log_error() { echo -e "\033[31m❌ $1\033[0m" >&2; }

# Ensure required labels exist
github_setup_labels() {
    log_info "Setting up GitHub labels for AI task management..."
    
    # Create status labels
    for status in "${!STATUS_LABELS[@]}"; do
        label="${STATUS_LABELS[$status]}"
        case $status in
            "todo") color="fbca04" ;; # yellow
            "in-progress") color="0052cc" ;; # blue  
            "done") color="0e8a16" ;; # green
            "blocked") color="d93f0b" ;; # red
        esac
        
        if ! gh label list | grep -q "^$label"; then
            gh label create "$label" --color "$color" --description "Task status: $status" || true
        fi
    done
    
    # Create priority labels
    for priority in "${!PRIORITY_LABELS[@]}"; do
        label="${PRIORITY_LABELS[$priority]}"
        case $priority in
            "high") color="b60205" ;; # dark red
            "medium") color="fbca04" ;; # yellow
            "low") color="c2e0c6" ;; # light green
        esac
        
        if ! gh label list | grep -q "^$label"; then
            gh label create "$label" --color "$color" --description "Task priority: $priority" || true
        fi
    done
    
    # Create agent labels
    for agent in "${!AGENT_LABELS[@]}"; do
        label="${AGENT_LABELS[$agent]}"
        case $agent in
            "bevy") color="ff6b35" ;; # orange
            "entt") color="5319e7" ;; # purple
            "flecs") color="f9ca24" ;; # gold
            "console") color="6c5ce7" ;; # indigo
            "meta") color="fd79a8" ;; # pink
            "orchestrator") color="74b9ff" ;; # light blue
        esac
        
        if ! gh label list | grep -q "^$label"; then
            gh label create "$label" --color "$color" --description "Assigned agent: $agent" || true
        fi
    done
    
    log_success "GitHub labels setup complete"
}

# Query Operations

# List tasks with filtering options
# Usage: github_list_tasks [status] [agent] [priority] [format]
# Format: json|summary|detailed (default: summary)
github_list_tasks() {
    local status_filter="${1:-}"
    local agent_filter="${2:-}"
    local priority_filter="${3:-}"
    local format="${4:-summary}"
    
    local label_filters=()
    
    # Build label filters
    if [[ -n "$status_filter" && "${STATUS_LABELS[$status_filter]:-}" ]]; then
        label_filters+=("${STATUS_LABELS[$status_filter]}")
    fi
    
    if [[ -n "$agent_filter" && "${AGENT_LABELS[$agent_filter]:-}" ]]; then
        label_filters+=("${AGENT_LABELS[$agent_filter]}")
    fi
    
    if [[ -n "$priority_filter" && "${PRIORITY_LABELS[$priority_filter]:-}" ]]; then
        label_filters+=("${PRIORITY_LABELS[$priority_filter]}")
    fi
    
    # Build query
    local query="is:issue is:open"
    for label in "${label_filters[@]}"; do
        query+=" label:\"$label\""
    done
    
    log_info "Querying tasks with filters: status=$status_filter agent=$agent_filter priority=$priority_filter"
    
    case "$format" in
        "json")
            gh issue list --search "$query" --json number,title,labels,assignees,state,createdAt,updatedAt
            ;;
        "detailed")
            gh issue list --search "$query" --json number,title,labels,assignees,state,createdAt,updatedAt,body | \
            jq -r '.[] | "=== TASK #\(.number): \(.title) ===\nStatus: \(.state)\nAssignees: \(if .assignees | length > 0 then (.assignees | map(.login) | join(", ")) else "None" end)\nLabels: \(.labels | map(.name) | join(", "))\nCreated: \(.createdAt)\nUpdated: \(.updatedAt)\nDescription:\n\(.body)\n"'
            ;;
        *)  # summary format
            echo "=== AVAILABLE TASKS ==="
            gh issue list --search "$query" --json number,title,labels,assignees,state | \
            jq -r '.[] | "Task #\(.number): \(.title) [\(.state)] - Assignee: \(if .assignees | length > 0 then (.assignees[0].login) else "None" end) - Labels: \(.labels | map(.name) | join(", "))"'
            ;;
    esac
}

# Get detailed information about a specific task
# Usage: github_get_task <issue_number> [format]
# Format: json|detailed (default: detailed)
github_get_task() {
    local issue_number="$1"
    local format="${2:-detailed}"
    
    if [[ -z "$issue_number" ]]; then
        log_error "Issue number required"
        return 1
    fi
    
    log_info "Getting task details for issue #$issue_number"
    
    case "$format" in
        "json")
            gh issue view "$issue_number" --json number,title,body,labels,assignees,state,createdAt,updatedAt,comments
            ;;
        *)  # detailed format
            echo "=== TASK #$issue_number DETAILS ==="
            gh issue view "$issue_number" --json number,title,body,labels,assignees,state,createdAt,updatedAt,comments | \
            jq -r '
                "Title: \(.title)",
                "Status: \(.state)",
                "Assignees: \(if .assignees | length > 0 then (.assignees | map(.login) | join(", ")) else "None" end)",
                "Labels: \(.labels | map(.name) | join(", "))",
                "Created: \(.createdAt)",
                "Updated: \(.updatedAt)",
                "",
                "=== DESCRIPTION ===",
                .body,
                "",
                "=== COMMENTS (\(.comments | length)) ===",
                (.comments | map("[\(.createdAt)] \(.author.login): \(.body)") | join("\n\n"))
            '
            ;;
    esac
}

# Query status and assignment information for a task
# Usage: github_query_status <issue_number>
github_query_status() {
    local issue_number="$1"
    
    if [[ -z "$issue_number" ]]; then
        log_error "Issue number required"
        return 1
    fi
    
    log_info "Querying status for task #$issue_number"
    
    gh issue view "$issue_number" --json number,title,labels,assignees,state | \
    jq -r '
        def extract_label_type(prefix): 
            .labels | map(.name) | map(select(startswith(prefix))) | map(ltrimstr(prefix)) | .[0] // "none";
        
        "=== TASK STATUS QUERY ===",
        "Issue: #\(.number) - \(.title)",
        "State: \(.state)",
        "Status: \(extract_label_type("status:"))",
        "Priority: \(extract_label_type("priority:"))",
        "Agent: \(extract_label_type("agent:"))",
        "Assignee: \(if .assignees | length > 0 then (.assignees[0].login) else "None" end)",
        "Coordination: \(if .assignees | length > 0 then "Assigned" else "Available" end)"
    '
}

# Write Operations

# Create a new task (GitHub Issue)
# Usage: github_create_task <title> <body> [priority] [agent] [assignee]
github_create_task() {
    local title="$1"
    local body="$2"
    local priority="${3:-medium}"
    local agent="${4:-}"
    local assignee="${5:-}"
    
    if [[ -z "$title" || -z "$body" ]]; then
        log_error "Title and body are required"
        return 1
    fi
    
    log_info "Creating new task: $title"
    
    # Build labels array
    local labels=("${STATUS_LABELS[todo]}" "${PRIORITY_LABELS[$priority]}")
    
    if [[ -n "$agent" && "${AGENT_LABELS[$agent]:-}" ]]; then
        labels+=("${AGENT_LABELS[$agent]}")
    fi
    
    # Create issue
    local issue_args=("--title" "$title" "--body" "$body")
    
    # Add labels
    for label in "${labels[@]}"; do
        issue_args+=("--label" "$label")
    done
    
    # Add assignee if specified
    if [[ -n "$assignee" ]]; then
        issue_args+=("--assignee" "$assignee")
    fi
    
    local issue_url
    issue_url=$(gh issue create "${issue_args[@]}")
    local issue_number
    issue_number=$(echo "$issue_url" | grep -o '[0-9]*$')
    
    log_success "Created task #$issue_number: $title"
    echo "Issue URL: $issue_url"
    echo "Issue Number: $issue_number"
    
    return 0
}

# Update task content
# Usage: github_update_task <issue_number> [title] [body]
github_update_task() {
    local issue_number="$1"
    local new_title="${2:-}"
    local new_body="${3:-}"
    
    if [[ -z "$issue_number" ]]; then
        log_error "Issue number required"
        return 1
    fi
    
    log_info "Updating task #$issue_number"
    
    local update_args=()
    
    if [[ -n "$new_title" ]]; then
        update_args+=("--title" "$new_title")
    fi
    
    if [[ -n "$new_body" ]]; then
        update_args+=("--body" "$new_body")
    fi
    
    if [[ ${#update_args[@]} -eq 0 ]]; then
        log_warning "No updates specified"
        return 0
    fi
    
    gh issue edit "$issue_number" "${update_args[@]}"
    log_success "Updated task #$issue_number"
}

# Assign or unassign a task
# Usage: github_assign_task <issue_number> <assignee|"none">
github_assign_task() {
    local issue_number="$1"
    local assignee="$2"
    
    if [[ -z "$issue_number" || -z "$assignee" ]]; then
        log_error "Issue number and assignee required"
        return 1
    fi
    
    if [[ "$assignee" == "none" ]]; then
        log_info "Unassigning task #$issue_number"
        gh issue edit "$issue_number" --remove-assignee "@me"
        log_success "Unassigned task #$issue_number"
    else
        log_info "Assigning task #$issue_number to $assignee"
        gh issue edit "$issue_number" --add-assignee "$assignee"
        log_success "Assigned task #$issue_number to $assignee"
    fi
}

# Status Operations

# Change task status (labels and state)
# Usage: github_change_status <issue_number> <new_status> [priority] [agent]
github_change_status() {
    local issue_number="$1"
    local new_status="$2"
    local new_priority="${3:-}"
    local new_agent="${4:-}"
    
    if [[ -z "$issue_number" || -z "$new_status" ]]; then
        log_error "Issue number and status required"
        return 1
    fi
    
    if [[ ! "${STATUS_LABELS[$new_status]:-}" ]]; then
        log_error "Invalid status: $new_status. Valid options: ${!STATUS_LABELS[*]}"
        return 1
    fi
    
    log_info "Changing status of task #$issue_number to $new_status"
    
    # Remove existing status labels
    for status_label in "${STATUS_LABELS[@]}"; do
        gh issue edit "$issue_number" --remove-label "$status_label" 2>/dev/null || true
    done
    
    # Add new status label
    gh issue edit "$issue_number" --add-label "${STATUS_LABELS[$new_status]}"
    
    # Update priority if specified
    if [[ -n "$new_priority" && "${PRIORITY_LABELS[$new_priority]:-}" ]]; then
        # Remove existing priority labels
        for priority_label in "${PRIORITY_LABELS[@]}"; do
            gh issue edit "$issue_number" --remove-label "$priority_label" 2>/dev/null || true
        done
        gh issue edit "$issue_number" --add-label "${PRIORITY_LABELS[$new_priority]}"
    fi
    
    # Update agent if specified
    if [[ -n "$new_agent" && "${AGENT_LABELS[$new_agent]:-}" ]]; then
        # Remove existing agent labels
        for agent_label in "${AGENT_LABELS[@]}"; do
            gh issue edit "$issue_number" --remove-label "$agent_label" 2>/dev/null || true
        done
        gh issue edit "$issue_number" --add-label "${AGENT_LABELS[$new_agent]}"
    fi
    
    # Close issue if status is "done"
    if [[ "$new_status" == "done" ]]; then
        gh issue close "$issue_number"
        log_success "Closed task #$issue_number (status: done)"
    else
        # Reopen if it was closed
        gh issue reopen "$issue_number" 2>/dev/null || true
        log_success "Updated task #$issue_number status to $new_status"
    fi
}

# Add a comment to track progress
# Usage: github_add_comment <issue_number> <comment>
github_add_comment() {
    local issue_number="$1"
    local comment="$2"
    
    if [[ -z "$issue_number" || -z "$comment" ]]; then
        log_error "Issue number and comment required"
        return 1
    fi
    
    log_info "Adding comment to task #$issue_number"
    gh issue comment "$issue_number" --body "$comment"
    log_success "Added comment to task #$issue_number"
}

# Close a task as complete
# Usage: github_close_task <issue_number> [completion_comment]
github_close_task() {
    local issue_number="$1"
    local completion_comment="${2:-Task completed successfully}"
    
    if [[ -z "$issue_number" ]]; then
        log_error "Issue number required"
        return 1
    fi
    
    log_info "Closing task #$issue_number as complete"
    
    # Change status to done (this will also close the issue)
    github_change_status "$issue_number" "done"
    
    # Add completion comment
    github_add_comment "$issue_number" "$completion_comment"
    
    log_success "Completed and closed task #$issue_number"
}

# AI Helper Functions

# Get AI-friendly task summary for coordination
# Usage: github_get_ai_summary [agent] [status]
github_get_ai_summary() {
    local agent_filter="${1:-}"
    local status_filter="${2:-}"
    
    echo "=== AI TASK COORDINATION SUMMARY ==="
    echo "Generated: $(date -Iseconds)"
    echo ""
    
    # Get task counts by status
    echo "=== TASK STATUS OVERVIEW ==="
    for status in todo in-progress done blocked; do
        local count
        count=$(github_list_tasks "$status" "$agent_filter" "" "json" | jq '. | length')
        echo "$status: $count tasks"
    done
    echo ""
    
    # Get tasks for specified filters
    echo "=== FILTERED TASK LIST ==="
    github_list_tasks "$status_filter" "$agent_filter" "" "summary"
    echo ""
    
    # Get assignment overview
    echo "=== ASSIGNMENT OVERVIEW ==="
    for agent in bevy entt flecs console meta orchestrator; do
        local assigned_count
        assigned_count=$(github_list_tasks "" "$agent" "" "json" | jq '. | length')
        echo "$agent: $assigned_count tasks assigned"
    done
}

# Validate task structure for AI consumption
# Usage: github_validate_ai_structure <issue_number>
github_validate_ai_structure() {
    local issue_number="$1"
    
    if [[ -z "$issue_number" ]]; then
        log_error "Issue number required"
        return 1
    fi
    
    echo "=== AI STRUCTURE VALIDATION FOR TASK #$issue_number ==="
    
    local task_data
    task_data=$(github_get_task "$issue_number" "json")
    
    # Check required fields
    local title body state labels
    title=$(echo "$task_data" | jq -r '.title')
    body=$(echo "$task_data" | jq -r '.body')
    state=$(echo "$task_data" | jq -r '.state')
    labels=$(echo "$task_data" | jq -r '.labels | map(.name) | join(", ")')
    
    echo "✓ Title: $title"
    echo "✓ Body: $(echo "$body" | wc -c) characters"
    echo "✓ State: $state"
    echo "✓ Labels: $labels"
    
    # Check for AI-required structure
    local has_status has_priority has_agent
    has_status=$(echo "$task_data" | jq -r '.labels | map(.name) | map(select(startswith("status:"))) | length > 0')
    has_priority=$(echo "$task_data" | jq -r '.labels | map(.name) | map(select(startswith("priority:"))) | length > 0')
    has_agent=$(echo "$task_data" | jq -r '.labels | map(.name) | map(select(startswith("agent:"))) | length > 0')
    
    echo ""
    echo "=== AI PARSING VALIDATION ==="
    echo "Status label: $(if [[ "$has_status" == "true" ]]; then echo "✓ Present"; else echo "✗ Missing"; fi)"
    echo "Priority label: $(if [[ "$has_priority" == "true" ]]; then echo "✓ Present"; else echo "✗ Missing"; fi)"
    echo "Agent label: $(if [[ "$has_agent" == "true" ]]; then echo "✓ Present"; else echo "✗ Missing"; fi)"
    
    # Check if body contains structured information
    if echo "$body" | grep -q "## Objective\|## Requirements\|## Acceptance Criteria"; then
        echo "Structured content: ✓ Present"
    else
        echo "Structured content: ✗ Missing structured sections"
    fi
    
    echo ""
    echo "=== AI READABILITY TEST ==="
    github_query_status "$issue_number"
}

# Command-line interface
main() {
    case "${1:-help}" in
        "setup")
            github_setup_labels
            ;;
        "list")
            github_list_tasks "${2:-}" "${3:-}" "${4:-}" "${5:-summary}"
            ;;
        "get")
            github_get_task "${2:-}" "${3:-detailed}"
            ;;
        "status")
            github_query_status "${2:-}"
            ;;
        "create")
            github_create_task "${2:-}" "${3:-}" "${4:-medium}" "${5:-}" "${6:-}"
            ;;
        "update")
            github_update_task "${2:-}" "${3:-}" "${4:-}"
            ;;
        "assign")
            github_assign_task "${2:-}" "${3:-}"
            ;;
        "change-status")
            github_change_status "${2:-}" "${3:-}" "${4:-}" "${5:-}"
            ;;
        "comment")
            github_add_comment "${2:-}" "${3:-}"
            ;;
        "close")
            github_close_task "${2:-}" "${3:-Task completed successfully}"
            ;;
        "ai-summary")
            github_get_ai_summary "${2:-}" "${3:-}"
            ;;
        "validate")
            github_validate_ai_structure "${2:-}"
            ;;
        "help"|*)
            echo "GitHub API Core Operations for AI Task Management"
            echo ""
            echo "Setup:"
            echo "  setup                           - Create required labels"
            echo ""
            echo "Query Operations:"
            echo "  list [status] [agent] [priority] [format] - List tasks with filters"
            echo "  get <issue#> [format]           - Get task details"
            echo "  status <issue#>                 - Get status and assignment info"
            echo ""
            echo "Write Operations:"
            echo "  create <title> <body> [priority] [agent] [assignee] - Create new task"
            echo "  update <issue#> [title] [body]  - Update task content"
            echo "  assign <issue#> <assignee>      - Assign/unassign task"
            echo ""
            echo "Status Operations:"
            echo "  change-status <issue#> <status> [priority] [agent] - Change task status"
            echo "  comment <issue#> <comment>      - Add progress comment"
            echo "  close <issue#> [comment]        - Close task as complete"
            echo ""
            echo "AI Helper Functions:"
            echo "  ai-summary [agent] [status]     - Get AI-friendly coordination summary"
            echo "  validate <issue#>               - Validate task structure for AI"
            echo ""
            echo "Status values: todo, in-progress, done, blocked"
            echo "Priority values: high, medium, low"
            echo "Agent values: bevy, entt, flecs, console, meta, orchestrator"
            ;;
    esac
}

# Execute main function if script is run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi