#!/bin/bash

# Mandatory Coordination Check Script
# Verifies that agents are following coordination protocols
# Usage: bash meta/scripts/mandatory-coordination-check.sh [agent-mode] [action]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [action]"
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo "Actions: validate, continue_development, work_on_task"
    echo ""
    echo "This script validates that agents follow mandatory coordination protocols."
    echo "It should be called before agents begin any task work."
    exit 1
}

# Validate input
if [[ $# -ne 2 ]]; then
    usage
fi

AGENT_MODE="$1"
ACTION="$2"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta")
        ;;
    *)
        echo -e "${RED}ERROR: Invalid agent mode '$AGENT_MODE'${NC}"
        usage
        ;;
esac

# Validate action
case "$ACTION" in
    "validate"|"continue_development"|"work_on_task")
        ;;
    *)
        echo -e "${RED}ERROR: Invalid action '$ACTION'${NC}"
        usage
        ;;
esac

# Determine task directory based on agent mode
case "$AGENT_MODE" in
    "bevy")
        TASK_DIR="$PROJECT_ROOT/gol-bevy/tasks"
        ;;
    "entt")
        TASK_DIR="$PROJECT_ROOT/gol-entt/tasks"
        ;;
    "flecs")
        TASK_DIR="$PROJECT_ROOT/gol-flecs/tasks"
        ;;
    "console")
        TASK_DIR="$PROJECT_ROOT/gol-console-client/tasks"
        ;;
    "meta")
        TASK_DIR="$PROJECT_ROOT/meta/tasks"
        ;;
esac

echo "=== MANDATORY COORDINATION CHECK ==="
echo "Agent Mode: $AGENT_MODE"
echo "Action: $ACTION"
echo "Task Directory: $TASK_DIR"
echo ""

# Function to check if any task is currently claimed by this agent
check_active_claim() {
    local active_claim=""
    local claimed_task=""
    
    for claim_file in "$TASK_DIR"/*.claimed; do
        if [[ -f "$claim_file" ]]; then
            # Extract agent info from claim file
            local claim_agent=$(grep "Agent:" "$claim_file" | cut -d' ' -f2)
            local claim_mode=$(grep "Mode:" "$claim_file" | cut -d' ' -f2)
            
            # Check if this is our agent mode
            if [[ "$claim_mode" == "$AGENT_MODE" ]]; then
                active_claim="$claim_file"
                claimed_task=$(basename "$claim_file" .claimed)
                break
            fi
        fi
    done
    
    echo "$active_claim|$claimed_task"
}

# Function to find next available task
find_next_task() {
    local next_task=""
    
    # Use the task analysis script to find recommended task
    if [[ -f "$SCRIPT_DIR/analyze-agent-tasks.sh" ]]; then
        local analysis_output=$(bash "$SCRIPT_DIR/analyze-agent-tasks.sh" "$AGENT_MODE" 2>/dev/null)
        next_task=$(echo "$analysis_output" | grep "🎯" | head -1 | awk '{print $2}')
    fi
    
    # Fallback: find any unclaimed .md file
    if [[ -z "$next_task" ]]; then
        for task_file in "$TASK_DIR"/*.md; do
            if [[ -f "$task_file" ]]; then
                local task_name=$(basename "$task_file")
                local claim_file="$task_file.claimed"
                
                if [[ ! -f "$claim_file" ]]; then
                    next_task="$task_name"
                    break
                fi
            fi
        done
    fi
    
    echo "$next_task"
}

# STEP 1: Check current agent state
echo "Checking current agent state..."
claim_info=$(check_active_claim)
IFS='|' read -r active_claim claimed_task <<< "$claim_info"

if [[ -n "$active_claim" ]]; then
    echo -e "${BLUE}ℹ️  Agent has active claim: $claimed_task${NC}"
    echo "Claim file: $active_claim"
    echo ""
    
    case "$ACTION" in
        "continue_development")
            echo -e "${GREEN}✅ COORDINATION CHECK PASSED${NC}"
            echo "Agent may continue work on: $claimed_task"
            echo ""
            echo "REMINDER: Clean up when task complete:"
            echo "1. rm \"$active_claim\""
            echo "2. bash meta/scripts/unregister-agent.sh $AGENT_MODE $claimed_task"
            exit 0
            ;;
        "work_on_task")
            echo -e "${GREEN}✅ COORDINATION CHECK PASSED${NC}"
            echo "Agent may continue work on: $claimed_task"
            echo ""
            echo "To work on a different task, first complete current task:"
            echo "1. Complete work on: $claimed_task"
            echo "2. rm \"$active_claim\""
            echo "3. bash meta/scripts/unregister-agent.sh $AGENT_MODE $claimed_task"
            echo "4. Then claim new task with: bash meta/scripts/enforce-coordination.sh $AGENT_MODE [new-task]"
            exit 0
            ;;
        "validate")
            echo -e "${GREEN}✅ COORDINATION CHECK PASSED${NC}"
            echo "Agent is properly coordinated with active task: $claimed_task"
            exit 0
            ;;
    esac
else
    echo -e "${YELLOW}⚠️  No active claim found for agent mode: $AGENT_MODE${NC}"
    echo ""
    
    case "$ACTION" in
        "continue_development")
            # Find next available task
            next_task=$(find_next_task)
            
            if [[ -n "$next_task" ]]; then
                echo -e "${BLUE}📋 Next available task: $next_task${NC}"
                echo ""
                echo "MANDATORY COORDINATION REQUIRED:"
                echo "bash meta/scripts/enforce-coordination.sh $AGENT_MODE $next_task"
                echo ""
                echo -e "${RED}❌ ENFORCEMENT FAILED: Agent must claim task before beginning work${NC}"
                exit 1
            else
                echo -e "${YELLOW}⚠️  No available tasks found${NC}"
                echo "Check task directory: $TASK_DIR"
                echo "All tasks may be claimed by other agents or completed."
                exit 1
            fi
            ;;
        "work_on_task")
            echo -e "${RED}❌ ENFORCEMENT FAILED: No task claimed${NC}"
            echo ""
            echo "MANDATORY COORDINATION REQUIRED:"
            echo "Use: bash meta/scripts/enforce-coordination.sh $AGENT_MODE [task-file]"
            echo ""
            echo "Available tasks:"
            bash "$SCRIPT_DIR/analyze-agent-tasks.sh" "$AGENT_MODE" 2>/dev/null | grep "•" | head -5
            exit 1
            ;;
        "validate")
            echo -e "${RED}❌ COORDINATION VIOLATION: No coordination protocol in effect${NC}"
            echo ""
            echo "Agent is not following mandatory coordination protocol."
            echo "All agents must claim tasks before beginning work."
            exit 1
            ;;
    esac
fi