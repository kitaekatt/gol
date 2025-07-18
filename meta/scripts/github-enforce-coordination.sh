#!/bin/bash

# GitHub Issues Coordination Protocol Enforcement Script
# Ensures agents follow mandatory coordination protocols using GitHub Issues
# Usage: bash meta/scripts/github-enforce-coordination.sh [agent-mode] [issue-number]

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
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [issue-number]"
    echo "Agent modes: bevy, entt, flecs, console, meta, orchestrator"
    echo "Example: $0 bevy 123"
    echo ""
    echo "This script enforces mandatory coordination protocol compliance using GitHub Issues."
    echo "It must be called before any agent begins work on a GitHub Issue."
    exit 1
}

# Validate input
if [[ $# -ne 2 ]]; then
    usage
fi

AGENT_MODE="$1"
ISSUE_NUMBER="$2"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta"|"orchestrator")
        ;;
    *)
        echo -e "${RED}ERROR: Invalid agent mode '$AGENT_MODE'${NC}"
        usage
        ;;
esac

# Validate issue number is numeric
if ! [[ "$ISSUE_NUMBER" =~ ^[0-9]+$ ]]; then
    echo -e "${RED}ERROR: Issue number must be numeric${NC}"
    usage
fi

echo "=== GITHUB COORDINATION PROTOCOL ENFORCEMENT ==="
echo "Agent Mode: $AGENT_MODE"
echo "Issue Number: #$ISSUE_NUMBER"
echo ""

# STEP 1: Validate issue exists and get current state
echo "Validating GitHub Issue..."
issue_data=$(github_get_task "$ISSUE_NUMBER" "json" 2>/dev/null)

if [[ -z "$issue_data" || "$issue_data" == "null" ]]; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: GitHub Issue #$ISSUE_NUMBER does not exist${NC}"
    exit 1
fi

# Extract issue information
issue_title=$(echo "$issue_data" | jq -r '.title')
issue_state=$(echo "$issue_data" | jq -r '.state')
current_assignees=$(echo "$issue_data" | jq -r '.assignees | map(.login) | join(", ")')
current_labels=$(echo "$issue_data" | jq -r '.labels | map(.name) | join(", ")')

echo "Issue: #$ISSUE_NUMBER - $issue_title"
echo "State: $issue_state"
echo "Current Assignees: ${current_assignees:-None}"
echo "Current Labels: $current_labels"
echo ""

# STEP 2: Check if issue is already assigned to someone else
if [[ -n "$current_assignees" && "$current_assignees" != "None" ]]; then
    # Check if current user is already assigned
    current_user=$(gh api user | jq -r '.login')
    if [[ "$current_assignees" == *"$current_user"* ]]; then
        echo -e "${GREEN}✅ Already assigned to you ($current_user)${NC}"
    else
        echo -e "${RED}❌ ENFORCEMENT FAILED: Issue is already assigned to: $current_assignees${NC}"
        echo ""
        echo "RECOVERY ACTION: This issue is being worked on by another user."
        echo "Either wait for completion or coordinate with the assigned user."
        exit 1
    fi
else
    # STEP 3: Assign the issue to current user
    echo "Assigning issue to current user..."
    current_user=$(gh api user | jq -r '.login')
    
    if ! github_assign_task "$ISSUE_NUMBER" "$current_user"; then
        echo -e "${RED}❌ ENFORCEMENT FAILED: Could not assign issue${NC}"
        exit 1
    fi
fi

# STEP 4: Check and update agent labels
echo "Checking agent label assignment..."
agent_label="${AGENT_LABELS[$AGENT_MODE]}"

if [[ "$current_labels" != *"$agent_label"* ]]; then
    echo "Adding agent label: $agent_label"
    if ! gh issue edit "$ISSUE_NUMBER" --add-label "$agent_label"; then
        echo -e "${YELLOW}⚠️  WARNING: Could not add agent label${NC}"
        echo "Proceeding with assignment only."
    fi
fi

# STEP 5: Update status to in-progress if it's still todo
current_status=""
if [[ "$current_labels" == *"status:todo"* ]]; then
    current_status="todo"
elif [[ "$current_labels" == *"status:in-progress"* ]]; then
    current_status="in-progress"
elif [[ "$current_labels" == *"status:done"* ]]; then
    current_status="done"
elif [[ "$current_labels" == *"status:blocked"* ]]; then
    current_status="blocked"
fi

if [[ "$current_status" == "todo" || -z "$current_status" ]]; then
    echo "Updating status to in-progress..."
    if ! github_change_status "$ISSUE_NUMBER" "in-progress" "" "$AGENT_MODE"; then
        echo -e "${YELLOW}⚠️  WARNING: Could not update status${NC}"
        echo "Proceeding with assignment only."
    fi
elif [[ "$current_status" == "done" ]]; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: Issue is already completed${NC}"
    exit 1
fi

# STEP 6: Add initial progress comment
echo "Adding coordination comment..."
coordination_comment="🤖 **Agent Coordination**

Agent \`$AGENT_MODE\` has been assigned to work on this task.

**Coordination Details:**
- **Agent Mode:** $AGENT_MODE
- **Assigned User:** $current_user
- **Status:** in-progress
- **Timestamp:** $(date -Iseconds)

Work will begin shortly. Progress updates will be posted here."

if ! github_add_comment "$ISSUE_NUMBER" "$coordination_comment"; then
    echo -e "${YELLOW}⚠️  WARNING: Could not add coordination comment${NC}"
    echo "Proceeding without comment."
fi

# STEP 7: Success - provide cleanup instructions
echo ""
echo -e "${GREEN}✅ GITHUB COORDINATION PROTOCOL ENFORCEMENT PASSED${NC}"
echo ""
echo "Agent '$AGENT_MODE' is now authorized to work on GitHub Issue #$ISSUE_NUMBER"
echo ""
echo "MANDATORY CLEANUP WHEN TASK COMPLETE:"
echo "1. Update status to done: bash meta/scripts/github-core-ops.sh change-status $ISSUE_NUMBER done"
echo "2. Add completion comment: bash meta/scripts/github-core-ops.sh comment $ISSUE_NUMBER \"Task completed by agent $AGENT_MODE\""
echo "3. Unassign if needed: bash meta/scripts/github-core-ops.sh assign $ISSUE_NUMBER none"
echo ""
echo "GITHUB COMMANDS FOR TASK WORK:"
echo "- Get task details: bash meta/scripts/github-core-ops.sh get $ISSUE_NUMBER"
echo "- Add progress update: bash meta/scripts/github-core-ops.sh comment $ISSUE_NUMBER \"[your update]\""
echo "- Update status: bash meta/scripts/github-core-ops.sh change-status $ISSUE_NUMBER [status]"
echo ""
echo "Task work may now begin..."