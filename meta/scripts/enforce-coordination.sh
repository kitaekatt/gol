#!/bin/bash

# Coordination Protocol Enforcement Script
# Ensures agents follow mandatory coordination protocols
# Usage: bash meta/scripts/enforce-coordination.sh [agent-mode] [task-file]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
COORDINATION_DIR="$PROJECT_ROOT/meta/coordination"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [task-file]"
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo "Example: $0 bevy setup-project.md"
    echo ""
    echo "This script enforces mandatory coordination protocol compliance."
    echo "It must be called before any agent begins work on a task."
    exit 1
}

# Validate input
if [[ $# -ne 2 ]]; then
    usage
fi

AGENT_MODE="$1"
TASK_FILE="$2"

# Validate agent mode
case "$AGENT_MODE" in
    "bevy"|"entt"|"flecs"|"console"|"meta")
        ;;
    *)
        echo -e "${RED}ERROR: Invalid agent mode '$AGENT_MODE'${NC}"
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

TASK_PATH="$TASK_DIR/$TASK_FILE"
CLAIM_FILE="$TASK_PATH.claimed"

echo "=== COORDINATION PROTOCOL ENFORCEMENT ==="
echo "Agent Mode: $AGENT_MODE"
echo "Task File: $TASK_FILE"
echo "Task Path: $TASK_PATH"
echo ""

# STEP 1: Validate task file exists
if [[ ! -f "$TASK_PATH" ]]; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: Task file does not exist${NC}"
    echo "Expected: $TASK_PATH"
    exit 1
fi

# STEP 2: Check if task is already claimed
if [[ -f "$CLAIM_FILE" ]]; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: Task is already claimed${NC}"
    echo "Claim file exists: $CLAIM_FILE"
    echo ""
    echo "Claim details:"
    cat "$CLAIM_FILE"
    echo ""
    echo "RECOVERY ACTION: Another agent is working on this task."
    echo "Either wait for completion or coordinate with the active agent."
    exit 1
fi

# STEP 3: Validate coordination system is available
if [[ ! -d "$COORDINATION_DIR" ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Coordination directory missing${NC}"
    echo "Creating coordination directory: $COORDINATION_DIR"
    mkdir -p "$COORDINATION_DIR"
fi

# STEP 4: Check for conflicting agents using Python coordination system
echo "Checking for coordination conflicts..."
COORDINATION_CHECK_RESULT=$(python3 "$PROJECT_ROOT/meta/coordination/coordination-system.py" check_conflicts "$AGENT_MODE" "$TASK_PATH" 2>/dev/null || echo "CONFLICT_CHECK_FAILED")

if [[ "$COORDINATION_CHECK_RESULT" == "CONFLICT_CHECK_FAILED" ]]; then
    echo -e "${YELLOW}⚠️  WARNING: Coordination system check failed${NC}"
    echo "Proceeding with basic claim file enforcement only."
elif [[ "$COORDINATION_CHECK_RESULT" == "CONFLICTS_DETECTED" ]]; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: Coordination conflicts detected${NC}"
    echo "Run 'python3 meta/coordination/coordination-system.py status' for details."
    exit 1
fi

# STEP 5: Claim the task
echo "Claiming task..."
if ! bash "$SCRIPT_DIR/claim-task.sh" "$AGENT_MODE" "$TASK_FILE"; then
    echo -e "${RED}❌ ENFORCEMENT FAILED: Could not claim task${NC}"
    exit 1
fi

# STEP 6: Register with coordination system
echo "Registering with coordination system..."
if ! bash "$SCRIPT_DIR/register-agent.sh" "$AGENT_MODE" "$TASK_FILE"; then
    echo -e "${YELLOW}⚠️  WARNING: Agent registration failed${NC}"
    echo "Task is claimed but not registered with coordination system."
    echo "Proceeding with claim file enforcement only."
fi

# STEP 7: Success - provide cleanup instructions
echo ""
echo -e "${GREEN}✅ COORDINATION PROTOCOL ENFORCEMENT PASSED${NC}"
echo ""
echo "Agent '$AGENT_MODE' is now authorized to work on '$TASK_FILE'"
echo ""
echo "MANDATORY CLEANUP WHEN TASK COMPLETE:"
echo "1. Remove claim file: rm \"$CLAIM_FILE\""
echo "2. Unregister agent: bash meta/scripts/unregister-agent.sh $AGENT_MODE $TASK_FILE"
echo ""
echo "FAILURE TO CLEAN UP WILL BLOCK OTHER AGENTS FROM WORKING ON THIS TASK"
echo ""
echo "Task work may now begin..."