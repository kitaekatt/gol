#!/bin/bash

# Agent Task Analysis Script
# Provides comprehensive task analysis for any agent in a single tool call
# Usage: bash meta/scripts/analyze-agent-tasks.sh [agent-mode]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"

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
    echo "Agent modes: bevy, entt, flecs, console, meta, all"
    echo "Examples: $0 bevy"
    echo "         $0 all"
    exit 1
}

# Validate input
if [[ $# -ne 1 ]]; then
    usage
fi

AGENT_MODE="$1"

# Validate agent mode and determine tasks directory
case "$AGENT_MODE" in
    "bevy")
        TASKS_DIR="$PROJECT_ROOT/gol-bevy/tasks"
        ;;
    "entt")
        TASKS_DIR="$PROJECT_ROOT/gol-entt/tasks"
        ;;
    "flecs")
        TASKS_DIR="$PROJECT_ROOT/gol-flecs/tasks"
        ;;
    "console")
        TASKS_DIR="$PROJECT_ROOT/gol-console-client/tasks"
        ;;
    "meta")
        TASKS_DIR="$PROJECT_ROOT/meta/tasks"
        ;;
    "all")
        TASKS_DIR="all"
        ;;
    *)
        echo "Error: Invalid agent mode '$AGENT_MODE'"
        usage
        ;;
esac

# Check if tasks directory exists (skip for 'all' mode)
if [[ "$AGENT_MODE" != "all" ]] && [[ ! -d "$TASKS_DIR" ]]; then
    echo "Error: Tasks directory not found: $TASKS_DIR"
    exit 1
fi

# Function to parse task metadata from file
parse_task_metadata() {
    local file="$1"
    local filename=$(basename "$file")
    
    # Initialize default values
    local priority="Medium"
    local duration="Unknown"
    local parallel_safe="Unknown"
    local conflicts=""
    local dependencies=""
    local description=""
    local status="Available"
    
    # Parse metadata from file
    if [[ -f "$file" ]]; then
        # Extract priority
        if grep -q "Priority.*High" "$file" 2>/dev/null; then
            priority="High"
        elif grep -q "Priority.*Low" "$file" 2>/dev/null; then
            priority="Low"
        elif grep -q "Priority.*Medium" "$file" 2>/dev/null; then
            priority="Medium"
        fi
        
        # Extract estimated duration/effort
        duration=$(grep -i "estimated.*effort\|duration\|estimated effort" "$file" 2>/dev/null | head -1 | sed 's/.*: *//; s/\*\*//g' | cut -c1-20 || echo "Unknown")
        
        # Extract parallel safety
        if grep -q "Parallel Safe.*true\|Parallel Safe.*Yes" "$file" 2>/dev/null; then
            parallel_safe="Yes"
        elif grep -q "Parallel Safe.*false\|Parallel Safe.*No" "$file" 2>/dev/null; then
            parallel_safe="No"
        fi
        
        # Extract conflicts
        conflicts=$(grep "Conflicts With" "$file" 2>/dev/null | sed 's/.*: *//; s/\[//; s/\]//; s/\*\*//g' | cut -c1-30 || echo "")
        
        # Extract dependencies
        dependencies=$(grep "Depends On" "$file" 2>/dev/null | sed 's/.*: *//; s/\[//; s/\]//; s/\*\*//g' | cut -c1-30 || echo "")
        
        # Extract description (first line after # title)
        description=$(grep -A 5 "^# " "$file" 2>/dev/null | grep -v "^#" | grep -v "^$" | head -1 | cut -c1-60 || echo "")
        
        # Check if task is completed
        if grep -q "TASK COMPLETED\|COMPLETED\|Status.*Complete" "$file" 2>/dev/null; then
            status="Completed"
        fi
    fi
    
    echo "$filename|$priority|$duration|$parallel_safe|$conflicts|$dependencies|$description|$status"
}

# Function to check coordination status
check_coordination_status() {
    local active_agents=0
    local file_locks=0
    local coordination_status="CLEAR"
    
    # Check active agents
    if [[ -f "$COORDINATION_DIR/active-agents.json" ]]; then
        active_agents=$(jq -r '.agents | length' "$COORDINATION_DIR/active-agents.json" 2>/dev/null || echo "0")
    fi
    
    # Check file locks
    if [[ -f "$COORDINATION_DIR/file-locks.json" ]]; then
        file_locks=$(jq -r '.locks | length' "$COORDINATION_DIR/file-locks.json" 2>/dev/null || echo "0")
    fi
    
    if [[ $active_agents -gt 0 ]] || [[ $file_locks -gt 0 ]]; then
        coordination_status="CONFLICTS"
    fi
    
    echo "$coordination_status|$active_agents|$file_locks"
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

# Function to print task line
print_task() {
    local task_data="$1"
    IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status <<< "$task_data"
    
    local conflict_text="none"
    if [[ -n "$conflicts" ]]; then
        conflict_text="$conflicts"
    fi
    
    printf "  • %-25s [%s] [Parallel: %s] [Conflicts: %s]\n" "$filename" "$duration" "$parallel_safe" "$conflict_text"
    if [[ -n "$description" ]]; then
        printf "    %s\n" "$description"
    fi
}

# Function to analyze tasks for a specific agent
analyze_agent_tasks() {
    local agent_mode="$1"
    local tasks_dir="$2"
    local coord_status="$3"
    
    declare -a high_priority_tasks=()
    declare -a medium_priority_tasks=()
    declare -a low_priority_tasks=()
    declare -a completed_tasks=()
    declare -a blocked_tasks=()
    
    # Find all .md files in tasks directory
    while IFS= read -r -d '' task_file; do
        if [[ -f "$task_file" ]]; then
            task_data=$(parse_task_metadata "$task_file")
            IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status <<< "$task_data"
            
            case "$status" in
                "Completed")
                    completed_tasks+=("$task_data")
                    ;;
                *)
                    # Check if task is blocked by conflicts
                    # Only block if there are ACTUAL conflicts, not just coordination status
                    if [[ "$conflicts" != "None" ]] && [[ "$conflicts" != "none" ]] && [[ -n "$conflicts" ]] && [[ "$coord_status" == "CONFLICTS" ]]; then
                        blocked_tasks+=("$task_data|Coordination conflicts")
                    else
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
                    fi
                    ;;
            esac
        fi
    done < <(find "$tasks_dir" -name "*.md" -type f -not -path "*/completed/*" -print0 2>/dev/null)
    
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
    
    # Store agent summary
    agent_task_counts["$agent_mode"]="$((${#high_priority_tasks[@]} + ${#medium_priority_tasks[@]} + ${#low_priority_tasks[@]}))"
    
}

# Check coordination status
coordination_info=$(check_coordination_status)
IFS='|' read -r coord_status active_count lock_count <<< "$coordination_info"

if [[ "$AGENT_MODE" == "all" ]]; then
    # Unified analysis mode
    echo "=== UNIFIED PROJECT TASK ANALYSIS ==="
    echo "Coordination Status: $coord_status"
    echo "Total Active Agents: $active_count"
    echo ""
    
    # Global arrays for all agents
    declare -a all_high_priority_tasks=()
    declare -a all_medium_priority_tasks=()
    declare -a all_low_priority_tasks=()
    declare -a all_completed_tasks=()
    declare -a all_blocked_tasks=()
    declare -A agent_task_counts=()
    
    # Analyze all agents
    analyze_agent_tasks "bevy" "$PROJECT_ROOT/gol-bevy/tasks" "$coord_status"
    analyze_agent_tasks "entt" "$PROJECT_ROOT/gol-entt/tasks" "$coord_status"
    analyze_agent_tasks "flecs" "$PROJECT_ROOT/gol-flecs/tasks" "$coord_status"
    analyze_agent_tasks "console" "$PROJECT_ROOT/gol-console-client/tasks" "$coord_status"
    analyze_agent_tasks "meta" "$PROJECT_ROOT/meta/tasks" "$coord_status"
    
    # Print highest priority tasks across all agents
    echo "=== HIGHEST PRIORITY TASKS (Cross-Agent) ==="
    
    if [[ ${#all_high_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${RED}🔴 HIGH PRIORITY${NC}"
        for task in "${all_high_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode filename priority duration parallel_safe conflicts dependencies description status <<< "$task"
            printf "  • [%s] %-25s [%s] - %s\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$filename" "$duration" "Foundation work"
        done
        echo ""
    fi
    
    if [[ ${#all_medium_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${YELLOW}🟡 MEDIUM PRIORITY${NC}"
        for task in "${all_medium_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode filename priority duration parallel_safe conflicts dependencies description status <<< "$task"
            printf "  • [%s] %-25s [%s] - %s\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$filename" "$duration" "Development work"
        done
        echo ""
    fi
    
    if [[ ${#all_low_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${GREEN}🟢 LOW PRIORITY${NC}"
        for task in "${all_low_priority_tasks[@]}"; do
            IFS='|' read -r agent_mode filename priority duration parallel_safe conflicts dependencies description status <<< "$task"
            printf "  • [%s] %-25s [%s] - %s\n" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$filename" "$duration" "Enhancement work"
        done
        echo ""
    fi
    
    # Print agent summary
    echo "=== AGENT SUMMARY ==="
    for agent in bevy entt flecs console meta; do
        count=${agent_task_counts[$agent]:-0}
        printf "%s: %d tasks available\n" "$(echo "$agent" | tr '[:lower:]' '[:upper:]')" "$count"
    done
    echo ""
    
    # Recommend orchestrator action
    echo "=== RECOMMENDED ORCHESTRATOR ACTION ==="
    if [[ ${#all_high_priority_tasks[@]} -gt 0 ]]; then
        # Get first high priority task
        IFS='|' read -r agent_mode filename priority duration parallel_safe conflicts dependencies description status <<< "${all_high_priority_tasks[0]}"
        printf "Delegate %s to %s agent (%s, highest impact)\n" "$filename" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$duration"
    elif [[ ${#all_medium_priority_tasks[@]} -gt 0 ]]; then
        # Get first medium priority task
        IFS='|' read -r agent_mode filename priority duration parallel_safe conflicts dependencies description status <<< "${all_medium_priority_tasks[0]}"
        printf "Delegate %s to %s agent (%s, development ready)\n" "$filename" "$(echo "$agent_mode" | tr '[:lower:]' '[:upper:]')" "$duration"
    else
        echo "No tasks available for delegation"
    fi
    
else
    # Single agent analysis mode
    echo "=== AGENT TASK ANALYSIS: $(echo "$AGENT_MODE" | tr '[:lower:]' '[:upper:]') ==="
    
    echo "Coordination Status: $coord_status"
    echo "Active Agents: $active_count"
    echo "File Locks: $lock_count"
    echo ""
    
    # Find and analyze all tasks
    declare -a high_priority_tasks=()
    declare -a medium_priority_tasks=()
    declare -a low_priority_tasks=()
    declare -a completed_tasks=()
    declare -a blocked_tasks=()
    
    # Find all .md files in tasks directory
    while IFS= read -r -d '' task_file; do
        if [[ -f "$task_file" ]]; then
            task_data=$(parse_task_metadata "$task_file")
            IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status <<< "$task_data"
            
            case "$status" in
                "Completed")
                    completed_tasks+=("$task_data")
                    ;;
                *)
                    # Check if task is blocked by conflicts
                    # Only block if there are ACTUAL conflicts, not just coordination status
                    if [[ "$conflicts" != "None" ]] && [[ "$conflicts" != "none" ]] && [[ -n "$conflicts" ]] && [[ "$coord_status" == "CONFLICTS" ]]; then
                        blocked_tasks+=("$task_data|Coordination conflicts")
                    else
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
                    fi
                    ;;
            esac
        fi
    done < <(find "$TASKS_DIR" -name "*.md" -type f -not -path "*/completed/*" -print0)

    # Print available tasks
    echo "=== AVAILABLE TASKS ==="

    if [[ ${#high_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${RED}🔴 HIGH PRIORITY${NC}"
        for task in "${high_priority_tasks[@]}"; do
            print_task "$task"
        done
        echo ""
    fi

    if [[ ${#medium_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${YELLOW}🟡 MEDIUM PRIORITY${NC}"
        for task in "${medium_priority_tasks[@]}"; do
            print_task "$task"
        done
        echo ""
    fi

    if [[ ${#low_priority_tasks[@]} -gt 0 ]]; then
        echo -e "${GREEN}🟢 LOW PRIORITY${NC}"
        for task in "${low_priority_tasks[@]}"; do
            print_task "$task"
        done
        echo ""
    fi

    # Print blocked tasks
    if [[ ${#blocked_tasks[@]} -gt 0 ]]; then
        echo "=== BLOCKED TASKS ==="
        for task in "${blocked_tasks[@]}"; do
            IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status reason <<< "$task"
            printf "  • %-25s - Blocked by: %s\n" "$filename" "$reason"
        done
        echo ""
    fi

    # Print completed tasks
    if [[ ${#completed_tasks[@]} -gt 0 ]]; then
        echo "=== COMPLETED TASKS ==="
        for task in "${completed_tasks[@]}"; do
            IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status <<< "$task"
            printf "  • %-25s - Status: %s\n" "$filename" "$status"
        done
        echo ""
    fi

    # Recommend next task
    echo "=== RECOMMENDED NEXT TASK ==="
    recommended_task=""
    recommendation_reason=""

    if [[ ${#high_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${high_priority_tasks[0]}"
        recommendation_reason="Highest priority task available"
    elif [[ ${#medium_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${medium_priority_tasks[0]}"
        recommendation_reason="Medium priority task with no conflicts"
    elif [[ ${#low_priority_tasks[@]} -gt 0 ]]; then
        recommended_task="${low_priority_tasks[0]}"
        recommendation_reason="Low priority task available"
    fi

    if [[ -n "$recommended_task" ]]; then
        IFS='|' read -r filename priority duration parallel_safe conflicts dependencies description status <<< "$recommended_task"
        echo -e "${BLUE}🎯 $filename${NC}"
        echo "   Priority: $priority"
        echo "   Duration: $duration"
        echo "   Parallel Safe: $parallel_safe"
        echo "   Conflicts: ${conflicts:-none}"
        echo "   Rationale: $recommendation_reason"
    else
        echo "No tasks available for execution"
    fi

    echo ""

    # Summary
    echo "=== AGENT DECISION SUMMARY ==="
    total_available=$((${#high_priority_tasks[@]} + ${#medium_priority_tasks[@]} + ${#low_priority_tasks[@]}))
    echo "Total Available: $total_available"
    echo "Coordination Status: $coord_status"
    echo "Resource Availability: $(if [[ $coord_status == "CLEAR" ]]; then echo "available"; else echo "conflicts"; fi)"

    if [[ -n "$recommended_task" ]]; then
        echo "Recommendation: Begin work on $filename"
        echo ""
        echo "=== AGENT REGISTRATION COMMAND ==="
        echo "To register as active agent, run:"
        echo "bash meta/scripts/register-agent.sh $AGENT_MODE $filename"
        echo ""
        echo "To unregister when task complete, run:"
        echo "bash meta/scripts/unregister-agent.sh $AGENT_MODE $filename"
    else
        echo "Recommendation: No tasks available - request new tasks or switch modes"
    fi
fi