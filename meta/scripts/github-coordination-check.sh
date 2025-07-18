#!/bin/bash

# GitHub Issues Mandatory Coordination Check Script
# Verifies that agents are following coordination protocols using GitHub Issues
# Usage: bash meta/scripts/github-coordination-check.sh [agent-mode] [action]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source the GitHub core operations
source "$SCRIPT_DIR/github-core-ops.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [action]"
    echo "Agent modes: bevy, entt, flecs, console, meta, orchestrator"
    echo "Actions: validate, continue_development, work_on_task"
    echo ""
    echo "This script validates that agents follow mandatory coordination protocols using GitHub Issues."
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
    "bevy"|"entt"|"flecs"|"console"|"meta"|"orchestrator")
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

echo "=== GITHUB MANDATORY COORDINATION CHECK ==="
echo "Agent Mode: $AGENT_MODE"
echo "Action: $ACTION"
echo ""

# Function to check if agent has any assigned GitHub Issues in progress
check_active_github_assignment() {
    local current_user assigned_issues
    
    current_user=$(gh api user | jq -r '.login' 2>/dev/null || echo "unknown")
    
    # Get all issues assigned to current user with agent label and in-progress status
    local agent_label="${AGENT_LABELS[$AGENT_MODE]}"
    assigned_issues=$(gh issue list --assignee "$current_user" --label "$agent_label" --label "status:in-progress" --json number,title,labels --limit 50)
    
    local active_issue=""
    local issue_count=0
    
    if [[ "$assigned_issues" != "[]" ]]; then
        issue_count=$(echo "$assigned_issues" | jq '. | length')
        if [[ $issue_count -gt 0 ]]; then
            # Get the first in-progress issue
            active_issue=$(echo "$assigned_issues" | jq -r '.[0] | "#\(.number): \(.title)"')
            active_issue_number=$(echo "$assigned_issues" | jq -r '.[0].number')
        fi
    fi
    
    echo "$active_issue|$active_issue_number|$issue_count"
}

# Function to find next available GitHub Issue
find_next_github_issue() {
    local next_issue=""
    local next_issue_number=""
    
    # Use the GitHub task analysis script to find recommended task
    if [[ -f "$SCRIPT_DIR/github-analyze-agent-tasks.sh" ]]; then
        local analysis_output=$(bash "$SCRIPT_DIR/github-analyze-agent-tasks.sh" "$AGENT_MODE" 2>/dev/null)
        
        # Extract issue number from the recommendation line
        next_issue_number=$(echo "$analysis_output" | grep "🎯" | head -1 | grep -o '#[0-9]*' | tr -d '#')
        
        if [[ -n "$next_issue_number" ]]; then
            next_issue=$(echo "$analysis_output" | grep "🎯" | head -1 | sed 's/.*🎯 //')
        fi
    fi
    
    # Fallback: find any unassigned issue with agent label
    if [[ -z "$next_issue_number" ]]; then
        local agent_label="${AGENT_LABELS[$AGENT_MODE]}"
        local available_issues=$(gh issue list --label "$agent_label" --label "status:todo" --json number,title --limit 10)
        
        if [[ "$available_issues" != "[]" ]]; then
            next_issue_number=$(echo "$available_issues" | jq -r '.[0].number')
            next_issue=$(echo "$available_issues" | jq -r '.[0] | "#\(.number): \(.title)"')
        fi
    fi
    
    echo "$next_issue|$next_issue_number"
}

# STEP 1: Check current agent GitHub assignment state
echo "Checking current GitHub assignment state..."
assignment_info=$(check_active_github_assignment)
IFS='|' read -r active_issue active_issue_number issue_count <<< "$assignment_info"

if [[ -n "$active_issue" && "$active_issue" != "" ]]; then
    echo -e "${BLUE}ℹ️  Agent has active GitHub assignment: $active_issue${NC}"
    if [[ $issue_count -gt 1 ]]; then
        echo -e "${YELLOW}⚠️  Multiple in-progress issues found ($issue_count total)${NC}"
        echo "Consider focusing on one issue at a time for better coordination."
    fi
    echo ""
    
    case "$ACTION" in
        "continue_development")
            echo -e "${GREEN}✅ GITHUB COORDINATION CHECK PASSED${NC}"
            echo "Agent may continue work on: $active_issue"
            echo ""
            echo "GITHUB COMMANDS FOR CONTINUED WORK:"
            echo "- Get task details: bash meta/scripts/github-core-ops.sh get $active_issue_number"
            echo "- Add progress update: bash meta/scripts/github-core-ops.sh comment $active_issue_number \"[your update]\""
            echo "- Complete task: bash meta/scripts/github-core-ops.sh change-status $active_issue_number done"
            exit 0
            ;;
        "work_on_task")
            echo -e "${GREEN}✅ GITHUB COORDINATION CHECK PASSED${NC}"
            echo "Agent may continue work on: $active_issue"
            echo ""
            echo "To work on a different GitHub Issue, first complete current issue:"
            echo "1. Complete work on: $active_issue"
            echo "2. bash meta/scripts/github-core-ops.sh change-status $active_issue_number done"
            echo "3. bash meta/scripts/github-core-ops.sh comment $active_issue_number \"Task completed by agent $AGENT_MODE\""
            echo "4. Then claim new issue with: bash meta/scripts/github-enforce-coordination.sh $AGENT_MODE [new-issue-number]"
            exit 0
            ;;
        "validate")
            echo -e "${GREEN}✅ GITHUB COORDINATION CHECK PASSED${NC}"
            echo "Agent is properly coordinated with active GitHub Issue: $active_issue"
            exit 0
            ;;
    esac
else
    echo -e "${YELLOW}⚠️  No active GitHub assignment found for agent mode: $AGENT_MODE${NC}"
    echo ""
    
    case "$ACTION" in
        "continue_development")
            # Find next available GitHub Issue
            issue_info=$(find_next_github_issue)
            IFS='|' read -r next_issue next_issue_number <<< "$issue_info"
            
            if [[ -n "$next_issue_number" ]]; then
                echo -e "${BLUE}📋 Next available GitHub Issue: $next_issue${NC}"
                echo ""
                echo "MANDATORY GITHUB COORDINATION REQUIRED:"
                echo "bash meta/scripts/github-enforce-coordination.sh $AGENT_MODE $next_issue_number"
                echo ""
                echo -e "${RED}❌ ENFORCEMENT FAILED: Agent must claim GitHub Issue before beginning work${NC}"
                exit 1
            else
                echo -e "${YELLOW}⚠️  No available GitHub Issues found for agent: $AGENT_MODE${NC}"
                echo "All issues may be assigned to other users or completed."
                echo ""
                echo "CREATE NEW ISSUE:"
                echo "bash meta/scripts/github-core-ops.sh create \"[Title]\" \"[Description]\" medium $AGENT_MODE"
                exit 1
            fi
            ;;
        "work_on_task")
            echo -e "${RED}❌ ENFORCEMENT FAILED: No GitHub Issue assigned${NC}"
            echo ""
            echo "MANDATORY GITHUB COORDINATION REQUIRED:"
            echo "Use: bash meta/scripts/github-enforce-coordination.sh $AGENT_MODE [issue-number]"
            echo ""
            echo "Available GitHub Issues:"
            bash "$SCRIPT_DIR/github-analyze-agent-tasks.sh" "$AGENT_MODE" 2>/dev/null | grep "•" | head -5
            exit 1
            ;;
        "validate")
            echo -e "${RED}❌ GITHUB COORDINATION VIOLATION: No coordination protocol in effect${NC}"
            echo ""
            echo "Agent is not following mandatory GitHub coordination protocol."
            echo "All agents must be assigned to GitHub Issues before beginning work."
            echo ""
            echo "AVAILABLE ACTIONS:"
            echo "1. Find and assign available issue: bash meta/scripts/github-analyze-agent-tasks.sh $AGENT_MODE"
            echo "2. Enforce coordination: bash meta/scripts/github-enforce-coordination.sh $AGENT_MODE [issue-number]"
            echo "3. Create new issue: bash meta/scripts/github-core-ops.sh create \"[Title]\" \"[Description]\" medium $AGENT_MODE"
            exit 1
            ;;
    esac
fi