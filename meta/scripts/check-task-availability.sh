#!/bin/bash

# Task Availability Checker
# Checks if a task is available for work or already claimed
# Usage: bash meta/scripts/check-task-availability.sh [agent-mode] [task-file]

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
    echo "UNAVAILABLE: Task file '$TASK_PATH' does not exist"
    exit 1
fi

# Check if task is already claimed
CLAIM_FILE="$TASK_PATH.claimed"
if [[ -f "$CLAIM_FILE" ]]; then
    echo "CLAIMED: Task '$TASK_FILE' is already claimed"
    echo "Claim details:"
    cat "$CLAIM_FILE"
    exit 1
else
    echo "AVAILABLE: Task '$TASK_FILE' is available for work"
    echo "Task path: $TASK_PATH"
    echo ""
    echo "To claim this task, run:"
    echo "bash meta/scripts/claim-task.sh $AGENT_MODE $TASK_FILE"
fi