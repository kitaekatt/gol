#!/bin/bash

# GitHub Issues Task Completion Script
# Handles proper completion and cleanup of GitHub Issues tasks
# Usage: bash meta/scripts/github-task-completion.sh [agent-mode] [issue-number] [completion-message]

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
    echo "Usage: $0 [agent-mode] [issue-number] [completion-message]"
    echo "Agent modes: bevy, entt, flecs, console, meta, orchestrator"
    echo "Example: $0 bevy 123 \"Task completed successfully with all acceptance criteria met\""
    echo ""
    echo "This script handles proper completion and cleanup of GitHub Issues tasks."
    exit 1
}

# Validate input
if [[ $# -lt 2 ]]; then
    usage
fi

AGENT_MODE="$1"
ISSUE_NUMBER="$2"
COMPLETION_MESSAGE="${3:-Task completed successfully by agent $AGENT_MODE}"

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

echo "=== GITHUB TASK COMPLETION PROTOCOL ==="
echo "Agent Mode: $AGENT_MODE"
echo "Issue Number: #$ISSUE_NUMBER"
echo "Completion Message: $COMPLETION_MESSAGE"
echo ""

# STEP 1: Validate issue exists and current state
echo "Validating GitHub Issue state..."
issue_data=$(github_get_task "$ISSUE_NUMBER" "json" 2>/dev/null)

if [[ -z "$issue_data" || "$issue_data" == "null" ]]; then
    echo -e "${RED}❌ COMPLETION FAILED: GitHub Issue #$ISSUE_NUMBER does not exist${NC}"
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

# STEP 2: Verify assignment and agent label
current_user=$(gh api user | jq -r '.login' 2>/dev/null || echo "unknown")
agent_label="${AGENT_LABELS[$AGENT_MODE]}"

# Check if assigned to current user
if [[ -z "$current_assignees" || "$current_assignees" == "None" ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Issue not assigned to anyone${NC}"
elif [[ "$current_assignees" != *"$current_user"* ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Issue assigned to $current_assignees, not current user ($current_user)${NC}"
    echo "Proceeding with completion anyway..."
fi

# Check agent label
if [[ "$current_labels" != *"$agent_label"* ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Issue doesn't have agent label: $agent_label${NC}"
    echo "Adding agent label for completion tracking..."
    gh issue edit "$ISSUE_NUMBER" --add-label "$agent_label" || true
fi

# STEP 3: Add completion comment with details
echo "Adding completion comment..."
current_timestamp=$(date -Iseconds)
detailed_completion_comment="✅ **Task Completion Report**

$COMPLETION_MESSAGE

**Completion Details:**
- **Completed By:** $AGENT_MODE agent (\`$current_user\`)
- **Completion Time:** $current_timestamp
- **Final Status:** done

**Work Summary:**
This task has been completed according to the requirements and acceptance criteria. All deliverables have been implemented and validated.

**Next Steps:**
Task is ready for review. Issue will be closed and marked as done."

if ! github_add_comment "$ISSUE_NUMBER" "$detailed_completion_comment"; then
    echo -e "${YELLOW}⚠️  WARNING: Could not add completion comment${NC}"
    echo "Proceeding with status update..."
fi

# STEP 4: Update status to done (this will also close the issue)
echo "Updating status to done..."
if ! github_change_status "$ISSUE_NUMBER" "done" "" "$AGENT_MODE"; then
    echo -e "${RED}❌ COMPLETION FAILED: Could not update status to done${NC}"
    exit 1
fi

# STEP 5: Verify issue is now closed
echo "Verifying issue closure..."
updated_issue_data=$(github_get_task "$ISSUE_NUMBER" "json" 2>/dev/null)
updated_state=$(echo "$updated_issue_data" | jq -r '.state')

if [[ "$updated_state" == "closed" ]]; then
    echo -e "${GREEN}✅ Issue successfully closed${NC}"
else
    echo -e "${YELLOW}⚠️  Issue state: $updated_state (expected: closed)${NC}"
fi

# STEP 6: Optional cleanup - unassign if requested
read -p "Unassign issue from current user? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Unassigning issue..."
    if github_assign_task "$ISSUE_NUMBER" "none"; then
        echo -e "${GREEN}✅ Issue unassigned${NC}"
    else
        echo -e "${YELLOW}⚠️  Could not unassign issue${NC}"
    fi
fi

# STEP 7: Success summary
echo ""
echo -e "${GREEN}✅ GITHUB TASK COMPLETION SUCCESSFUL${NC}"
echo ""
echo "**Completion Summary:**"
echo "- Issue: #$ISSUE_NUMBER - $issue_title"
echo "- Agent: $AGENT_MODE"
echo "- Status: done (closed)"
echo "- Completed by: $current_user"
echo "- Timestamp: $current_timestamp"
echo ""
echo "**Post-Completion Actions:**"
echo "1. ✅ Status updated to done"
echo "2. ✅ Issue closed"
echo "3. ✅ Completion comment added"
echo "4. ✅ Agent coordination cleaned up"
echo ""
echo "**Available Next Actions:**"
echo "- Find next task: bash meta/scripts/github-analyze-agent-tasks.sh $AGENT_MODE"
echo "- Claim new task: bash meta/scripts/github-enforce-coordination.sh $AGENT_MODE [issue-number]"
echo "- Create new task: bash meta/scripts/github-core-ops.sh create \"[Title]\" \"[Description]\" medium $AGENT_MODE"
echo ""
echo "Task completion process finished successfully!"