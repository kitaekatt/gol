#!/bin/bash

# Task Claiming Script
# Claims a task to prevent conflicts with other agents
# Usage: bash meta/scripts/claim-task.sh [agent-mode] [task-file]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Usage function
usage() {
    echo "Usage: $0 [agent-mode] [task-file]"
    echo "Agent modes: bevy, entt, flecs, console, meta"
    echo "Example: $0 bevy setup-project.md"
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
        echo "Error: Invalid agent mode '$AGENT_MODE'"
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

# Check if task file exists
TASK_PATH="$TASK_DIR/$TASK_FILE"
if [[ ! -f "$TASK_PATH" ]]; then
    echo "Error: Task file '$TASK_PATH' does not exist"
    exit 1
fi

# Check if task is already claimed
CLAIM_FILE="$TASK_PATH.claimed"
if [[ -f "$CLAIM_FILE" ]]; then
    echo "Error: Task '$TASK_FILE' is already claimed"
    echo "Claim file: $CLAIM_FILE"
    cat "$CLAIM_FILE"
    exit 1
fi

# Generate agent ID and timestamp
TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
AGENT_ID="${AGENT_MODE}_$(date +%s)"

# Create claim file
cat > "$CLAIM_FILE" << EOF
TASK CLAIMED
Agent: $AGENT_ID
Mode: $AGENT_MODE
Task: $TASK_FILE
Start Time: $TIMESTAMP
Status: IN_PROGRESS

This file indicates that the task is currently being worked on.
Other agents should not work on this task until this claim file is removed.
EOF

echo "Task claimed successfully: $TASK_FILE"
echo "Agent: $AGENT_ID"
echo "Claim file: $CLAIM_FILE"
echo ""
echo "IMPORTANT: Remove claim file when task is complete:"
echo "rm \"$CLAIM_FILE\""