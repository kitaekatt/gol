#!/bin/bash

# GitHub Agent Task Analysis Script
# Provides comprehensive GitHub Issues task analysis for any agent in a single tool call
# Usage: bash meta/scripts/github-analyze-agent-tasks.sh [agent-mode]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source the GitHub core operations
source "$SCRIPT_DIR/github-core-ops.sh"

# Colors for output (if terminal supports it)
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if running in terminal that supports colors
if [[ ! -t 1 ]]; then
    RED=""
    YELLOW=""
    GREEN=""
    BLUE=""
    NC=""
fi

# Usage function
usage() {
    echo "Usage: $0 [agent-mode|all]"
    echo "Agent modes: bevy, entt, flecs, console, meta, orchestrator, all"
    echo "Examples: $0 bevy"
    echo "         $0 all"
    exit 1
}

# Validate input
if [[ $# -ne 1 ]]; then
    usage
fi

AGENT_MODE="$1"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta"|"orchestrator"|"all")
        # Valid modes
        ;;
    *)
        echo "Error: Invalid agent mode '$AGENT_MODE'"
        usage
        ;;
esac

# Function to parse GitHub Issue data for AI consumption
parse_github_task() {
    local issue_json="$1"
    
    # Extract basic info
    local number title state assignee labels
    number=$(echo "$issue_json" | jq -r '.number')
    title=$(echo "$issue_json" | jq -r '.title')
    state=$(echo "$issue_json" | jq -r '.state')
    assignee=$(echo "$issue_json" | jq -r 'if .assignees | length > 0 then .assignees[0].login else "None" end')
    
    # Extract labels and parse priority/status/agent
    local priority="Medium"
    local status="todo"
    local agent=""
    
    labels=$(echo "$issue_json" | jq -r '.labels | map(.name) | join(",")')
    
    # Parse priority from labels
    if [[ "$labels" == *"priority:high"* ]]; then
        priority="High"
    elif [[ "$labels" == *"priority:low"* ]]; then
        priority="Low"
    fi
    
    # Parse status from labels
    if [[ "$labels" == *"status:in-progress"* ]]; then
        status="in-progress"
    elif [[ "$labels" == *"status:done"* ]]; then
        status="done"
    elif [[ "$labels" == *"status:blocked"* ]]; then
        status="blocked"
    fi
    
    # Parse agent from labels
    for agent_type in bevy entt flecs console meta orchestrator; do
        if [[ "$labels" == *"agent:$agent_type"* ]]; then
            agent="$agent_type"
            break
        fi
    done
    
    # Determine coordination status
    local coordination="Available"
    if [[ "$assignee" != "None" ]]; then
        coordination="Assigned"
    fi
    
    echo "$number|$title|$priority|$status|$agent|$assignee|$coordination|$state"
}

# Function to check GitHub coordination status
check_github_coordination_status() {
    local in_progress_count assigned_count
    
    # Count in-progress tasks
    in_progress_count=$(github_list_tasks "in-progress" "" "" "json" | jq '. | length')
    
    # Count assigned tasks
    assigned_count=$(github_list_tasks "" "" "" "json" | jq '[.[] | select(.assignees | length > 0)] | length')
    
    local coordination_status="CLEAR"
    if [[ $in_progress_count -gt 2 ]] || [[ $assigned_count -gt 5 ]]; then
        coordination_status="HEAVY_LOAD"
    fi
    
    echo "$coordination_status|$in_progress_count|$assigned_count"
}

# Function to get priority order value
get_priority_order() {
    case "$1" in
        "High") echo "1" ;;
        "Medium") echo "2" ;;
        "Low") echo "3" ;;
        *) echo "4" ;;
    esac
}

# Function to print GitHub task line
print_github_task() {
    local task_data="$1"
    IFS='|' read -r number title priority status agent assignee coordination state <<< "$task_data"
    
    local status_icon
    case "$status" in
        "todo") status_icon="📋" ;;
        "in-progress") status_icon="🔄" ;;
        "done") status_icon="✅" ;;
        "blocked") status_icon="🚫" ;;
        *) status_icon="❓" ;;
    esac
    
    printf "  • #%-4s %-30s %s [%s] [Assignee: %s]\n" "$number" "$title" "$status_icon" "$priority" "$assignee"
}

# Function to analyze GitHub tasks for a specific agent
analyze_github_agent_tasks() {
    local agent_mode="$1"
    
    declare -a high_priority_tasks=()
    declare -a medium_priority_tasks=()
    declare -a low_priority_tasks=()
    declare -a completed_tasks=()
    declare -a blocked_tasks=()
    declare -a in_progress_tasks=()
    
    # Get all GitHub Issues for this agent
    local issues_json
    if [[ "$agent_mode" == "all" ]]; then
        issues_json=$(github_list_tasks "" "" "" "json")
    else
        issues_json=$(github_list_tasks "" "$agent_mode" "" "json")
    fi
    
    # Parse each issue
    while IFS= read -r issue; do
        if [[ -n "$issue" && "$issue" != "null" ]]; then
            task_data=$(parse_github_task "$issue")
            IFS='|' read -r number title priority status agent assignee coordination state <<< "$task_data"
            
            # Skip if agent filter doesn't match (for "all" mode)
            if [[ "$agent_mode" != "all" && "$agent" != "$agent_mode" ]]; then
                continue
            fi
            
            case "$status" in
                "done")
                    completed_tasks+=("$task_data")
                    ;;
                "blocked")
                    blocked_tasks+=("$task_data")
                    ;;
                "in-progress")
                    in_progress_tasks+=("$task_data")
                    ;;
                *)
                    case "$priority" in
                        "High")
                            high_priority_tasks+=("$task_data")
                            ;;
                        "Medium")
                            medium_priority_tasks+=("$task_data")
                            ;;
                        "Low")
                            low_priority_tasks+=("$task_data")
                            ;;
                        *)
                            medium_priority_tasks+=("$task_data")
                            ;;
                    esac
                    ;;
            esac
        fi
    done < <(echo "$issues_json" | jq -c '.[]')
    
    # Store results in global arrays with agent prefix
    for task in "${high_priority_tasks[@]}"; do
        all_high_priority_tasks+=("$agent_mode|$task")
    done
    for task in "${medium_priority_tasks[@]}"; do
        all_medium_priority_tasks+=("$agent_mode|$task")
    done
    for task in "${low_priority_tasks[@]}"; do
        all_low_priority_tasks+=("$agent_mode|$task")
    done
    for task in "${completed_tasks[@]}"; do
        all_completed_tasks+=("$agent_mode|$task")
    done
    for task in "${blocked_tasks[@]}"; do
        all_blocked_tasks+=("$agent_mode|$task")
    done
    for task in "${in_progress_tasks[@]}"; do
        all_in_progress_tasks+=("$agent_mode|$task")
    done
    
    # Store agent summary
    agent_task_counts["$agent_mode"]="$((${#high_priority_tasks[@]} + ${#medium_priority_tasks[@]} + ${#low_priority_tasks[@]}))"
}

# Check GitHub coordination status
coordination_info=$(check_github_coordination_status)
IFS='|' read -r coord_status in_progress_count assigned_count <<< "$coordination_info"

if [[ "$AGENT_MODE" == "all" ]]; then
    # Unified analysis mode
    echo "=== UNIFIED GITHUB PROJECT TASK ANALYSIS ==="
    echo "Coordination Status: $coord_status"
    echo "In-Progress Tasks: $in_progress_count"
    echo "Assigned Tasks: $assigned_count"
    echo ""
    
    # Global arrays for all agents
    declare -a all_high_priority_tasks=()
    declare -a all_medium_priority_tasks=()
    declare -a all_low_priority_tasks=()
    declare -a all_completed_tasks=()
    declare -a all_blocked_tasks=()
    declare -a all_in_progress_tasks=()
    declare -A agent_task_counts=()
    
    # Analyze all agents
    for agent in bevy entt flecs console meta orchestrator; do
        analyze_github_agent_tasks "$agent"
    done
    
    # Print highest priority tasks across all agents
    echo "=== HIGHEST PRIORITY GITHUB ISSUES (Cross-Agent) ==="
    
    if [[ ${#all_high_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${RED}🔴 HIGH PRIORITY${NC}"
        for task in "${all_high_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "$task"
            printf "  • [%s] #%-4s %-30s [%s] - Foundation work\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$number" "$title" "$priority"
        done
        echo ""
    fi
    
    if [[ ${#all_medium_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${YELLOW}🟡 MEDIUM PRIORITY${NC}"
        for task in "${all_medium_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "$task"
            printf "  • [%s] #%-4s %-30s [%s] - Development work\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$number" "$title" "$priority"
        done
        echo ""
    fi
    
    if [[ ${#all_low_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${GREEN}🟢 LOW PRIORITY${NC}"
        for task in "${all_low_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "$task"
            printf "  • [%s] #%-4s %-30s [%s] - Enhancement work\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$number" "$title" "$priority"
        done
        echo ""
    fi
    
    # Print in-progress tasks
    if [[ ${#all_in_progress_tasks[@]} -gt 0 ]]; then
        echo "=== IN-PROGRESS GITHUB ISSUES ==="
        for task in "${all_in_progress_tasks[@]}"; do
            IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "$task"
            printf "  • [%s] #%-4s %-30s [%s] - Assignee: %s\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$number" "$title" "$priority" "$assignee"
        done
        echo ""
    fi
    
    # Print agent summary
    echo "=== AGENT SUMMARY ==="
    for agent in bevy entt flecs console meta orchestrator; do
        count=${agent_task_counts[$agent]:-0}
        printf "%s: %d GitHub Issues available\n" "$(echo "$agent" | tr '[:lower:]' '[:upper:]')" "$count"
    done
    echo ""
    
    # Recommend orchestrator action
    echo "=== RECOMMENDED ORCHESTRATOR ACTION ==="
    if [[ ${#all_high_priority_tasks[@]} -gt 0 ]]; then
        # Get first high priority task
        IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "${all_high_priority_tasks[0]}"
        printf "Delegate GitHub Issue #%s (%s) to %s agent (highest impact)\n" "$number" "$title" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')"
    elif [[ ${#all_medium_priority_tasks[@]} -gt 0 ]]; then
        # Get first medium priority task
        IFS='|' read -r agent_mode number title priority status agent assignee coordination state <<< "${all_medium_priority_tasks[0]}"
        printf "Delegate GitHub Issue #%s (%s) to %s agent (development ready)\n" "$number" "$title" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')"
    else
        echo "No GitHub Issues available for delegation"
    fi
    
else
    # Single agent analysis mode
    echo "=== GITHUB AGENT TASK ANALYSIS: $(echo "$AGENT_MODE" | tr '[:lower:]' '[:upper:]') ==="
    
    echo "Coordination Status: $coord_status"
    echo "In-Progress Tasks: $in_progress_count"
    echo "Assigned Tasks: $assigned_count"
    echo ""
    
    # Get all GitHub Issues for this agent
    local issues_json
    issues_json=$(github_list_tasks "" "$AGENT_MODE" "" "json")
    
    # Find and analyze all tasks
    declare -a high_priority_tasks=()
    declare -a medium_priority_tasks=()
    declare -a low_priority_tasks=()
    declare -a completed_tasks=()
    declare -a blocked_tasks=()
    declare -a in_progress_tasks=()
    
    # Parse each issue
    while IFS= read -r issue; do
        if [[ -n "$issue" && "$issue" != "null" ]]; then
            task_data=$(parse_github_task "$issue")
            IFS='|' read -r number title priority status agent assignee coordination state <<< "$task_data"
            
            case "$status" in
                "done")
                    completed_tasks+=("$task_data")
                    ;;
                "blocked")
                    blocked_tasks+=("$task_data")
                    ;;
                "in-progress")
                    in_progress_tasks+=("$task_data")
                    ;;
                *)
                    case "$priority" in
                        "High")
                            high_priority_tasks+=("$task_data")
                            ;;
                        "Medium")
                            medium_priority_tasks+=("$task_data")
                            ;;
                        "Low")
                            low_priority_tasks+=("$task_data")
                            ;;
                        *)
                            medium_priority_tasks+=("$task_data")
                            ;;
                    esac
                    ;;
            esac
        fi
    done < <(echo "$issues_json" | jq -c '.[]')

    # Print available tasks
    echo "=== AVAILABLE GITHUB ISSUES ==="

    if [[ ${#high_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${RED}🔴 HIGH PRIORITY${NC}"
        for task in "${high_priority_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    if [[ ${#medium_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${YELLOW}🟡 MEDIUM PRIORITY${NC}"
        for task in "${medium_priority_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    if [[ ${#low_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${GREEN}🟢 LOW PRIORITY${NC}"
        for task in "${low_priority_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    # Print in-progress tasks
    if [[ ${#in_progress_tasks[@]} -gt 0 ]]; then
        echo "=== IN-PROGRESS GITHUB ISSUES ==="
        for task in "${in_progress_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    # Print blocked tasks
    if [[ ${#blocked_tasks[@]} -gt 0 ]]; then
        echo "=== BLOCKED GITHUB ISSUES ==="
        for task in "${blocked_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    # Print completed tasks
    if [[ ${#completed_tasks[@]} -gt 0 ]]; then
        echo "=== COMPLETED GITHUB ISSUES ==="
        for task in "${completed_tasks[@]}"; do
            print_github_task "$task"
        done
        echo ""
    fi

    # Recommend next task
    echo "=== RECOMMENDED NEXT GITHUB ISSUE ==="
    recommended_task=""
    recommendation_reason=""

    if [[ ${#high_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${high_priority_tasks[0]}"
        recommendation_reason="Highest priority GitHub Issue available"
    elif [[ ${#medium_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${medium_priority_tasks[0]}"
        recommendation_reason="Medium priority GitHub Issue with no conflicts"
    elif [[ ${#low_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${low_priority_tasks[0]}"
        recommendation_reason="Low priority GitHub Issue available"
    fi

    if [[ -n "$recommended_task" ]]; then
        IFS='|' read -r number title priority status agent assignee coordination state <<< "$recommended_task"
        echo -e "${BLUE}🎯 GitHub Issue #$number: $title${NC}"
        echo "   Priority: $priority"
        echo "   Status: $status"
        echo "   Agent: $agent"
        echo "   Assignee: $assignee"
        echo "   Coordination: $coordination"
        echo "   Rationale: $recommendation_reason"
    else
        echo "No GitHub Issues available for execution"
    fi

    echo ""

    # Summary
    echo "=== GITHUB AGENT DECISION SUMMARY ==="
    total_available=$((${#high_priority_tasks[@]} + ${#medium_priority_tasks[@]} + ${#low_priority_tasks[@]}))
    echo "Total Available: $total_available"
    echo "Coordination Status: $coord_status"
    echo "Resource Availability: $(if [[ $coord_status == "CLEAR" ]]; then echo "available"; else echo "heavy load"; fi)"

    if [[ -n "$recommended_task" ]]; then
        echo "Recommendation: Begin work on GitHub Issue #$number"
        echo ""
        echo "=== GITHUB COORDINATION COMMANDS ==="
        echo "To assign issue to yourself:"
        echo "bash meta/scripts/github-core-ops.sh assign $number \$USER"
        echo ""
        echo "To change status to in-progress:"
        echo "bash meta/scripts/github-core-ops.sh change-status $number in-progress"
        echo ""
        echo "To add progress comment:"
        echo "bash meta/scripts/github-core-ops.sh comment $number \"Started work on this task\""
    else
        echo "Recommendation: No GitHub Issues available - create new issues or switch modes"
    fi
fi